#define VMA_IMPLEMENTATION
#include "VulkanCoreObjects.hpp"

#include "VkHooks.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"
#include "Graphics/Rendering/RenderController.hpp"
#include "Core/Assert.hpp"
#include "Utilities/MultiFuture.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "UIViewShaderManager.hpp"
#include "UI/Shaders.hpp"
#include "UIDrawUBO.hpp"

#include "glm/gtc/type_ptr.hpp"
#include "SDL3/SDL_vulkan.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

bool backendInWindowScope = false;

using ConditionFunction = std::function<bool(const vk::PhysicalDevice&)>;

namespace Cacao {
	Allocated<vk::Buffer> uiQuadBuffer;
	Allocated<vk::Buffer> uiQuadUBO;
	void* uiQuadUBOMem;
	std::shared_ptr<Material> uiQuadMat;

	//Sorts the list Vulkan physical devices by how many conditions each one satisfies
	void RankPhysicalDevices(std::vector<vk::PhysicalDevice>* devices, const std::vector<std::function<bool(const vk::PhysicalDevice&)>>& tests) {
		std::vector<int> scores(devices->size(), 0);

		//Run each test on each device
		for(size_t i = 0; i < devices->size(); ++i) {
			for(const auto& test : tests) {
				if(test((*devices)[i])) {
					scores[i]++;
				}
			}
		}

		//Sort devices in descending order based on scores
		std::sort(devices->begin(), devices->end(), [&scores, devices](const vk::PhysicalDevice& a, const vk::PhysicalDevice& b) {
			auto indexA = std::distance(devices->begin(), std::find(devices->begin(), devices->end(), a));
			auto indexB = std::distance(devices->begin(), std::find(devices->begin(), devices->end(), b));
			return scores[indexA] > scores[indexB];
		});
	}

	Immediate Immediate::Get() {
		//Check if we have an immediate for this thread already
		auto threadID = std::this_thread::get_id();
		if(immediates.contains(threadID)) {
			Immediate ret = immediates.at(threadID);
			ret.cmd.reset();
			return ret;
		}

		//Make a new immediate
		Immediate imm;
		try {
			vk::CommandPoolCreateInfo ipoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, 0);
			imm.pool = dev.createCommandPool(ipoolCI);
			vk::CommandBufferAllocateInfo allocCI(imm.pool, vk::CommandBufferLevel::ePrimary, 1);
			imm.cmd = dev.allocateCommandBuffers(allocCI)[0];
			imm.fence = dev.createFence({vk::FenceCreateFlagBits::eSignaled});
			immediates.insert_or_assign(threadID, imm);
		} catch(...) {
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), "Failed to create immediate object for thread!");
		}
		return imm;
	}

	void RenderController::Init() {
		CheckException(!isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot initialize the initialized render controller!");
		isInitialized = true;

		//Initialize base Vulkan functions
		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		//Get required layers
		std::vector<const char*> layers = {};
#ifdef DEBUG
		if(auto vv = std::getenv("CACAO_DISABLE_VULKAN_VALIDATION"); vv == nullptr || (vv != nullptr && std::string(vv).compare("YES") != 0)) layers.push_back("VK_LAYER_KHRONOS_validation");
		if(auto ad = std::getenv("CACAO_ENABLE_APIDUMP"); ad != nullptr && std::string(ad).compare("YES") == 0) layers.push_back("VK_LAYER_LUNARG_api_dump");
#endif

		//Get required extensions
		uint32_t extcount;
		const char* const* exts = SDL_Vulkan_GetExtensions(&extcount);
		EngineAssert(exts, "Could not load instance extension list!");
		std::vector<const char*> requiredInstanceExts(extcount);
		std::memcpy(requiredInstanceExts.data(), exts, extcount * sizeof(const char*));
#ifdef __APPLE__
		requiredInstanceExts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

		//Create instance
		vk::ApplicationInfo appInfo("Cacao Engine Client", 1, "Cacao Engine", 1, VK_API_VERSION_1_3);
		vk::InstanceCreateInfo instanceCI({}, &appInfo, layers, requiredInstanceExts);
#ifdef __APPLE__
		instanceCI.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif
		try {
			vk_instance = vk::createInstance(instanceCI);
		} catch(const vk::SystemError& err) {
			EngineAssert(false, std::string("Vulkan instance could not be created: ") + err.what());
		}

		//Initialize instance Vulkan functions
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vk_instance);

		//Find best device
		std::vector<const char*> requiredDevExts = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};
#ifdef __linux__
		requiredDevExts.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
#endif
		auto physicalDevices = vk_instance.enumeratePhysicalDevices();
		EngineAssert(!physicalDevices.empty(), "There are no Vulkan-compatible devices!");
		std::vector<vk::PhysicalDevice> okDevs;
		for(vk::PhysicalDevice& pdev : physicalDevices) {
			std::vector<vk::ExtensionProperties> availableExts = pdev.enumerateDeviceExtensionProperties();
			bool good = true;
			for(const char* ext : requiredDevExts) {
				bool found = false;
				for(vk::ExtensionProperties devExt : availableExts) {
					if(std::strncmp(devExt.extensionName, ext, std::strlen(ext)) == 0) {
						found = true;
						break;
					}
				}
				if(!found) {
					good = false;
					break;
				}
			}
			if(good) okDevs.push_back(pdev);
		}
		std::vector<std::function<bool(const vk::PhysicalDevice&)>> physDevChecks = {
			//Check for real GPU
			[](const vk::PhysicalDevice& device) {
				auto type = device.getProperties().deviceType;
				return type == vk::PhysicalDeviceType::eDiscreteGpu || type != vk::PhysicalDeviceType::eIntegratedGpu;
			},
			//Check for discrete GPU
			[](const vk::PhysicalDevice& device) {
				return device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
			}};
		RankPhysicalDevices(&okDevs, physDevChecks);
		physDev = okDevs[0];
		if(!physDev) {
			vk_instance.destroy();
			EngineAssert(false, "No devices support the required Vulkan extensions!");
		}
		Logging::EngineLog(std::string("Selected Vulkan device ") + std::string(physDev.getProperties().deviceName.data()), LogLevel::Trace);

		//Create logical device
		float qp = 1.0f;
		vk::DeviceQueueCreateInfo queueCI({}, 0, 1, &qp);
		vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures(VK_TRUE);
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures(VK_TRUE, &dynamicRenderingFeatures);
		vk::PhysicalDeviceSynchronization2Features sync2Features(VK_TRUE, &extendedDynamicStateFeatures);
		vk::PhysicalDeviceFeatures2 deviceFeatures2({}, &sync2Features);
		vk::DeviceCreateInfo deviceCI({}, queueCI, {}, requiredDevExts, nullptr, &deviceFeatures2);
		try {
			dev = physDev.createDevice(deviceCI);
		} catch(vk::SystemError& err) {
			vk_instance.destroy();
			EngineAssert(false, "The logical device could not be created!");
		}

		//Initialize device Vulkan functions
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vk_instance, dev);

		//Get graphics queue
		queue = dev.getQueue(0, 0);

		//Create memory allocator
		vma::VulkanFunctions vkFuncs(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetProcAddr, VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr);
		vma::AllocatorCreateInfo allocatorCI({}, physDev, dev, 0UL, 0, 0, 0, &vkFuncs, vk_instance, VK_API_VERSION_1_3);
		try {
			allocator = vma::createAllocator(allocatorCI);
		} catch(vk::SystemError& err) {
			dev.destroy();
			vk_instance.destroy();
			EngineAssert(false, "Could not create memory allocator!");
		}

		//Create command pool
		vk::CommandPoolCreateInfo rpoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 0);
		try {
			renderPool = dev.createCommandPool(rpoolCI);
		} catch(vk::SystemError& err) {
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			EngineAssert(false, "Could not create rendering command pool!");
		}

		//Make immediate for main thread
		Immediate imm = Immediate::Get();

		//Create globals UBO
		vk::BufferCreateInfo globalsCI({}, sizeof(glm::mat4) * 2, vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo globalsAllocCI({}, vma::MemoryUsage::eCpuToGpu);
		try {
			auto [globals, alloc] = allocator.createBuffer(globalsCI, globalsAllocCI);
			globalsUBO = {.alloc = alloc, .obj = globals};
		} catch(vk::SystemError& err) {
			Immediate::Cleanup();
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			std::stringstream emsg;
			emsg << "Could not create globals uniform buffer: " << err.what();
			EngineAssert(false, emsg.str());
		}

		//Map globals UBO
		if(allocator.mapMemory(globalsUBO.alloc, &globalsMem) != vk::Result::eSuccess) {
			allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
			Immediate::Cleanup();
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			EngineAssert(false, "Could not map globals uniform buffer memory!");
		}

		//Create UI quad globals UBO
		try {
			auto [globals, alloc] = allocator.createBuffer(globalsCI, globalsAllocCI);
			uiQuadUBO = {.alloc = alloc, .obj = globals};
		} catch(vk::SystemError& err) {
			allocator.unmapMemory(globalsUBO.alloc);
			allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
			Immediate::Cleanup();
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			std::stringstream emsg;
			emsg << "Could not create UI quad globals uniform buffer: " << err.what();
			EngineAssert(false, emsg.str());
		}

		//Map globals UBO
		if(allocator.mapMemory(uiQuadUBO.alloc, &uiQuadUBOMem) != vk::Result::eSuccess) {
			allocator.destroyBuffer(uiQuadUBO.obj, uiQuadUBO.alloc);
			allocator.unmapMemory(globalsUBO.alloc);
			allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
			Immediate::Cleanup();
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			EngineAssert(false, "Could not map UI quad globals uniform buffer memory!");
		}

		//Create UI drawing globals UBO
		try {
			auto [globals, alloc] = allocator.createBuffer(globalsCI, globalsAllocCI);
			uiUBO = {.alloc = alloc, .obj = globals};
		} catch(vk::SystemError& err) {
			allocator.unmapMemory(uiQuadUBO.alloc);
			allocator.destroyBuffer(uiQuadUBO.obj, uiQuadUBO.alloc);
			allocator.unmapMemory(globalsUBO.alloc);
			allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
			Immediate::Cleanup();
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			std::stringstream emsg;
			emsg << "Could not create UI drawing globals uniform buffer: " << err.what();
			EngineAssert(false, emsg.str());
		}

		//Map globals UBO
		if(allocator.mapMemory(uiUBO.alloc, &uiUBOMem) != vk::Result::eSuccess) {
			allocator.destroyBuffer(uiUBO.obj, uiUBO.alloc);
			allocator.unmapMemory(uiQuadUBO.alloc);
			allocator.destroyBuffer(uiQuadUBO.obj, uiQuadUBO.alloc);
			allocator.unmapMemory(globalsUBO.alloc);
			allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
			Immediate::Cleanup();
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			EngineAssert(false, "Could not map UI drawing globals uniform buffer memory!");
		}

		//Find good depth image format
		constexpr std::array<vk::Format, 3> allowedDepthFormats {{vk::Format::eD32Sfloat,
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD24UnormS8Uint}};
		for(vk::Format df : allowedDepthFormats) {
			vk::FormatProperties props = physDev.getFormatProperties(df);
			if((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) == vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
				selectedDF = df;
				break;
			}
		}
		if(selectedDF == vk::Format::eUndefined) {
			allocator.unmapMemory(uiUBO.alloc);
			allocator.destroyBuffer(uiUBO.obj, uiUBO.alloc);
			allocator.unmapMemory(uiQuadUBO.alloc);
			allocator.destroyBuffer(uiQuadUBO.obj, uiQuadUBO.alloc);
			allocator.unmapMemory(globalsUBO.alloc);
			allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
			Immediate::Cleanup();
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			EngineAssert(false, "Could not find any valid depth formats!");
		}

		//Create UI quad buffer
		constexpr float quadData[18] = {
			0.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 0.0f};
		auto vbsz = sizeof(float) * 18;
		vk::BufferCreateInfo vertexCI({}, vbsz, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer);
		vk::BufferCreateInfo vertexUpCI({}, vbsz, vk::BufferUsageFlagBits::eTransferSrc);
		vma::AllocationCreateInfo uploadAllocCI({}, vma::MemoryUsage::eCpuToGpu);
		vma::AllocationCreateInfo bufferAllocCI({}, vma::MemoryUsage::eGpuOnly);
		Allocated<vk::Buffer> vertexUp = {};
		{
			auto [buffer, alloc] = allocator.createBuffer(vertexCI, bufferAllocCI);
			uiQuadBuffer.alloc = alloc;
			uiQuadBuffer.obj = buffer;
		}
		{
			auto [buffer, alloc] = allocator.createBuffer(vertexUpCI, uploadAllocCI);
			vertexUp.alloc = alloc;
			vertexUp.obj = buffer;
		}
		void* gpuMem;
		allocator.mapMemory(vertexUp.alloc, &gpuMem);
		std::memcpy(gpuMem, quadData, vbsz);
		allocator.unmapMemory(vertexUp.alloc);
		vk::CommandBufferBeginInfo copyBegin {};
		imm.cmd.begin(copyBegin);
		{
			vk::BufferCopy2 copy(0UL, 0UL, vbsz);
			vk::CopyBufferInfo2 copyInfo(vertexUp.obj, uiQuadBuffer.obj, copy);
			imm.cmd.copyBuffer2(copyInfo);
		}
		imm.cmd.end();
		if(dev.getFenceStatus(imm.fence) == vk::Result::eSuccess) {
			vk::Result fenceWait = dev.waitForFences(imm.fence, VK_TRUE, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1000)).count());
			CheckException(fenceWait == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("WaitExpired"), "Waited too long for immediate fence reset!");
			dev.resetFences(imm.fence);
		}
		{
			vk::CommandBufferSubmitInfo cbsi(imm.cmd);
			vk::SubmitInfo2 si({}, {}, cbsi);
			SubmitCommandBuffer(si, imm.fence);
		}
		dev.waitForFences(imm.fence, VK_TRUE, UINT64_MAX);
		allocator.destroyBuffer(vertexUp.obj, vertexUp.alloc);

		//Create "null" texture
		vk::ImageCreateInfo nullCI({}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb, {1, 1, 1}, 1, 1, vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, 0);
		vma::AllocationCreateInfo nullAllocCI({}, vma::MemoryUsage::eGpuOnly, vk::MemoryPropertyFlagBits::eDeviceLocal);
		try {
			auto [img, alloc] = allocator.createImage(nullCI, nullAllocCI);
			nullImage = {.alloc = alloc, .obj = img};
		} catch(vk::SystemError& err) {
			allocator.unmapMemory(uiUBO.alloc);
			allocator.destroyBuffer(uiUBO.obj, uiUBO.alloc);
			allocator.unmapMemory(uiQuadUBO.alloc);
			allocator.destroyBuffer(uiQuadUBO.obj, uiQuadUBO.alloc);
			allocator.unmapMemory(globalsUBO.alloc);
			allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
			allocator.destroyBuffer(uiQuadBuffer.obj, uiQuadBuffer.alloc);
			Immediate::Cleanup();
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			std::stringstream emsg;
			emsg << "Could not create null texture image: " << err.what();
			EngineAssert(false, emsg.str());
		}

		//Create "null" buffer
		vk::BufferCreateInfo nullBufCI({}, 1, vk::BufferUsageFlagBits::eUniformBuffer);
		try {
			auto [buf, alloc] = allocator.createBuffer(nullBufCI, nullAllocCI);
			nullBuffer = {.alloc = alloc, .obj = buf};
		} catch(vk::SystemError& err) {
			allocator.destroyImage(nullImage.obj, nullImage.alloc);
			allocator.unmapMemory(uiUBO.alloc);
			allocator.destroyBuffer(uiUBO.obj, uiUBO.alloc);
			allocator.unmapMemory(uiQuadUBO.alloc);
			allocator.destroyBuffer(uiQuadUBO.obj, uiQuadUBO.alloc);
			allocator.unmapMemory(globalsUBO.alloc);
			allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
			allocator.destroyBuffer(uiQuadBuffer.obj, uiQuadBuffer.alloc);
			Immediate::Cleanup();
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			std::stringstream emsg;
			emsg << "Could not create null buffer: " << err.what();
			EngineAssert(false, emsg.str());
		}

		//Transition "null" image to shader read-only layout
		vk::CommandBufferBeginInfo nullImageStateBegin {};
		imm.cmd.reset();
		imm.cmd.begin(nullImageStateBegin);
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, nullImage.obj, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
			vk::DependencyInfo cdDI({}, {}, {}, barrier);
			imm.cmd.pipelineBarrier2(cdDI);
		}
		imm.cmd.end();
		if(dev.getFenceStatus(imm.fence) == vk::Result::eSuccess) {
			vk::Result fenceWait = dev.waitForFences(imm.fence, VK_TRUE, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1000)).count());
			CheckException(fenceWait == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("WaitExpired"), "Waited too long for immediate fence reset!");
			dev.resetFences(imm.fence);
		}
		{
			vk::CommandBufferSubmitInfo cbsi(imm.cmd);
			vk::SubmitInfo2 si({}, {}, cbsi);
			SubmitCommandBuffer(si, imm.fence);
		}
		dev.waitForFences(imm.fence, VK_TRUE, UINT64_MAX);

		//Create "null" image view
		vk::ImageViewCreateInfo nullViewCI({}, nullImage.obj, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Srgb,
			{vk::ComponentSwizzle::eZero, vk::ComponentSwizzle::eZero, vk::ComponentSwizzle::eZero, vk::ComponentSwizzle::eZero},
			{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
		nullView = dev.createImageView(nullViewCI);

		//Set up some basic variables
		didGenShaders = false;
		frameCycle = 0;
	}

	void Engine::RegisterBackendExceptions() {
		Exception::RegisterExceptionCode(100, "Vulkan");
		Exception::RegisterExceptionCode(101, "WaitExpired");
	}

	void SubmitCommandBuffer(vk::SubmitInfo2 submitInfo, vk::Fence fence) {
		std::lock_guard lk(queueMtx);
		queue.submit2(submitInfo, fence);
	}

	void RenderController::UpdateGraphicsState() {}

	void PreShaderCompileHook(Shader* shader) {
		//Set custom compile
		VkShaderData* sd = shaderDataLookup.at(shader);
		sd->usesCustomCompile = true;

		//Since we do custom compilation, we just do it here and let the "compile" call do nothing
		ShaderCompileSettings settings {};
		settings.blend = ShaderCompileSettings::Blending::One;
		settings.depth = ShaderCompileSettings::Depth::Off;
		settings.input = ShaderCompileSettings::InputType::VertexAndTexCoord;
		settings.wrapModes.insert_or_assign("glyph", vk::SamplerAddressMode::eClampToEdge);
		settings.wrapModes.insert_or_assign("image", vk::SamplerAddressMode::eClampToEdge);
		DoVkShaderCompile(sd, settings);
	}

	void UIViewShaderManager::PreCompileHook() {
		//Set custom compile
		VkShaderData* sd = shaderDataLookup.at(UIView::shader);
		sd->usesCustomCompile = true;

		//Since we do custom compilation, we just do it here and let the "compile" call do nothing
		ShaderCompileSettings settings {};
		settings.blend = ShaderCompileSettings::Blending::Src;
		settings.depth = ShaderCompileSettings::Depth::Off;
		settings.input = ShaderCompileSettings::InputType::VertexOnly;
		settings.wrapModes.insert_or_assign("uiTex", vk::SamplerAddressMode::eRepeat);
		DoVkShaderCompile(sd, settings);

		//Create the material for the UI quad
		uiQuadMat = uivsm->CreateMaterial();
	}

	void RenderController::WaitGPUIdleBeforeTerminate() {
		dev.waitIdle();
	}

	void Immediate::Cleanup() {
		//Wait for the device to be idle
		dev.waitIdle();

		for(auto imm : immediates) {
			dev.freeCommandBuffers(imm.second.pool, imm.second.cmd);
			dev.destroyCommandPool(imm.second.pool);
			dev.destroyFence(imm.second.fence);
		}
		immediates.clear();
	}

	void RenderController::Shutdown() {
		//Wait for the device to be idle so it's safe to destroy things
		dev.waitIdle();

		//Clean up immediate objects
		Immediate::Cleanup();

		//Release UI quad material
		uiQuadMat.reset();

		if(didGenShaders) {
			//Cleanup UI shaders
			DelShaders();
			uivsm.Release();
			didGenShaders = false;
		}

		//Destroy Vulkan objects
		allocator.destroyBuffer(nullBuffer.obj, nullBuffer.alloc);
		dev.destroyImageView(nullView);
		allocator.destroyImage(nullImage.obj, nullImage.alloc);
		allocator.unmapMemory(uiUBO.alloc);
		allocator.destroyBuffer(uiUBO.obj, uiUBO.alloc);
		allocator.unmapMemory(uiQuadUBO.alloc);
		allocator.destroyBuffer(uiQuadUBO.obj, uiQuadUBO.alloc);
		allocator.unmapMemory(globalsUBO.alloc);
		allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
		allocator.destroyBuffer(uiQuadBuffer.obj, uiQuadBuffer.alloc);
		dev.destroyImageView(depthView);
		allocator.destroyImage(depthImage.obj, depthImage.alloc);
		std::vector<vk::CommandBuffer> cbufs;
		for(auto f : frames) {
			dev.destroySemaphore(f.acquireSemaphore);
			dev.destroySemaphore(f.renderSemaphore);
			dev.destroyFence(f.fence);
			cbufs.push_back(f.cmd);
		}
		dev.freeCommandBuffers(renderPool, cbufs);
		dev.destroyCommandPool(renderPool);
		allocator.destroy();
		dev.destroy();
		vk_instance.destroy();
	}

	void RenderController::ProcessFrame(std::shared_ptr<Frame> frame) {
		//Skip if window is minimized; can cause Vulkan issues
		if(Window::Get()->IsMinimized()) {
			frameCycle = (frameCycle + 1) % frames.size();
			return;
		}

		//Fetch current Vulkan frame object
		VkFrame f = frames[frameCycle];
		activeFrame = &f;

		//Pre-process projection matrix to make it work with Vulkan
		glm::mat4 projMatrix = projectionCorrection * frame->projection;

		//Update globals UBO
		Shader::UploadCacaoGlobals(projMatrix, frame->view);

		//Acquire swapchain image
		unsigned int imgIdx;
	acquire:
		try {
			//Reset frame fence
			dev.waitForFences(f.fence, VK_TRUE, UINT64_MAX);
			dev.resetFences(f.fence);

			vk::AcquireNextImageInfoKHR acqInfo(swapchain, UINT64_MAX, f.acquireSemaphore, f.fence, 1);
			imgIdx = dev.acquireNextImage2KHR(acqInfo).value;
		} catch(vk::SystemError& err) {
			if(err.code() == vk::Result::eErrorOutOfDateKHR || err.code() == vk::Result::eSuboptimalKHR) {
				try {
					//Regen swapchain
					GenSwapchain();

					//Get new frame object (they get re-created)
					VkFrame f = frames[frameCycle];
					activeFrame = &f;

					goto acquire;
				} catch(std::exception& e) {
					std::stringstream emsg;
					emsg << "Failed to regenerate swapchain: " << e.what();
					CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str());
				}
			}
			std::stringstream emsg;
			emsg << "Failed to acquire swapchain image: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str());
		}

		//Wait for image acquisition
		dev.waitForFences(f.fence, VK_TRUE, UINT64_MAX);
		dev.resetFences(f.fence);

		//Reset frame command buffer
		vk::CommandBufferBeginInfo begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		f.cmd.reset({});
		f.cmd.begin(begin);

		//Transition image to drawable state
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 0, 0, images[imgIdx],
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			f.cmd.pipelineBarrier2(transition);
		}

		//Make sure depth image is drawable
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, 0, 0, depthImage.obj,
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			f.cmd.pipelineBarrier2(transition);
		}

		//Calculate extent
		vk::Extent2D extent;
		{
			glm::ivec2 winSize = Window::Get()->GetContentAreaSize();
			extent = {.width = (unsigned int)winSize.x, .height = (unsigned int)winSize.y};
			auto surfc = physDev.getSurfaceCapabilitiesKHR(surface);
			extent.width = std::clamp(extent.width, surfc.minImageExtent.width, surfc.maxImageExtent.width);
			extent.height = std::clamp(extent.height, surfc.minImageExtent.height, surfc.maxImageExtent.height);
		}

		//Set dynamic state
		vk::Viewport viewport(0.0f, 0.0f, float(extent.width), float(extent.height), 0.0f, 1.0f);
		vk::Rect2D scissor({0, 0}, extent);
		f.cmd.setViewport(0, viewport);
		f.cmd.setScissor(0, scissor);

		//Start rendering
		constexpr glm::vec3 clearColorSRGB {float(0xCF) / 256, 1.0f, float(0x4D) / 256};
		glm::vec3 clearColorLinear = glm::pow(clearColorSRGB, glm::vec3 {2.2f});
		vk::RenderingAttachmentInfo colorAttachment(imageViews[imgIdx], vk::ImageLayout::eColorAttachmentOptimal, {}, {}, {}, vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore, vk::ClearColorValue(std::array<float, 4> {clearColorLinear.r, clearColorLinear.g, clearColorLinear.b, 1.0f}));
		vk::RenderingAttachmentInfo depthAttachment(depthView, vk::ImageLayout::eDepthAttachmentOptimal, {}, {}, {}, vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore, vk::ClearDepthStencilValue(1.0f, 0.0f));
		vk::RenderingInfo renderingInfo({}, vk::Rect2D({0, 0}, extent), 1, 0, colorAttachment, &depthAttachment);
		f.cmd.beginRendering(renderingInfo);

		//Draw scene
		for(RenderObject& obj : frame->objects) {
			//Activate material
			obj.material.Activate();

			//Push transformation matrix
			f.cmd.pushConstants(activeShader->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), glm::value_ptr(obj.transformMatrix));

			//Draw the mesh
			obj.mesh->Draw();

			//Deactivate material
			obj.material.Deactivate();
		}

		//Draw skybox (if one exists)
		if(!frame->skybox.IsNull()) frame->skybox->Draw(frame->projection, frame->view);

		//Draw UI if it's been rendered
		if(Engine::Get()->GetGlobalUIView()->HasBeenRendered()) {
			//Create projection matrix
			glm::mat4 project = projectionCorrection * glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
			project[1][1] *= -1;//Flip
			project[3][1] -= 2; //Move into viewable area

			//Activate UI quad material
			uiQuadMat->WriteValue("uiTex", Engine::Get()->GetGlobalUIView());
			uiQuadMat->Activate();

			//Push identity transform matrix
			glm::mat4 identityTransform = glm::identity<glm::mat4>();
			f.cmd.pushConstants(activeShader->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), glm::value_ptr(identityTransform));

			//Set up the special UI quad globals uniform buffer
			glm::mat4 uiGlobalsData[2] = {project, glm::identity<glm::mat4>()};
			std::memcpy(uiQuadUBOMem, uiGlobalsData, 2 * sizeof(glm::mat4));
			vk::DescriptorBufferInfo uiGlobalsDBI(uiQuadUBO.obj, 0, vk::WholeSize);
			vk::WriteDescriptorSet dsWrite(VK_NULL_HANDLE, 0, 0, 1, vk::DescriptorType::eUniformBuffer, VK_NULL_HANDLE, &uiGlobalsDBI);
			f.cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, dsWrite);

			//Draw quad
			constexpr std::array<vk::DeviceSize, 1> offsets = {{0}};
			f.cmd.bindVertexBuffers(0, uiQuadBuffer.obj, offsets);
			f.cmd.draw(6, 1, 0, 0);

			//Deactivate UI quad material
			uiQuadMat->Deactivate();
		}

		//End rendering
		f.cmd.endRendering();

		//Transition image to sampleable state
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, 0, 0, images[imgIdx],
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			f.cmd.pipelineBarrier2(transition);
		}

		//End recording
		f.cmd.end();

		//Submit the command buffer to the queue
		vk::SemaphoreSubmitInfo semWait(f.acquireSemaphore, 0, vk::PipelineStageFlagBits2::eAllCommands);
		vk::CommandBufferSubmitInfo cbsi(f.cmd);
		vk::SemaphoreSubmitInfo semSignal(f.renderSemaphore, 0, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
		vk::SubmitInfo2 si({}, semWait, cbsi, semSignal);
		try {
			SubmitCommandBuffer(si, f.fence);
		} catch(vk::SystemError& err) {
			std::stringstream emsg;
			emsg << "Failed to submit frame command buffer: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str());
		}

		//Set submission for presenting
		submission = {.sem = f.renderSemaphore, .image = imgIdx};

		//Cycle the frame
		frameCycle = (frameCycle + 1) % frames.size();
	}

	void Window::UpdateVSyncState() {
		presentMode = (useVSync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate);
		try {
			GenSwapchain();
		} catch(vk::SystemError& err) {
			std::stringstream emsg;
			emsg << "Swapchain recreation failed: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str());
		}
	}

	void Window::Present() {
		CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot present to unopened window!");

		//Skip if window is minimized
		if(minimized) return;

		vk::PresentInfoKHR pi(submission.sem, swapchain, submission.image);
		try {
			queue.presentKHR(pi);
		} catch(vk::SystemError& err) {
			if(err.code() == vk::Result::eErrorOutOfDateKHR || err.code() == vk::Result::eSuboptimalKHR) {
				try {
					//Regen swapchain
					GenSwapchain();

					return;
				} catch(std::exception& e) {
					std::stringstream emsg;
					emsg << "Failed to regenerate swapchain: " << e.what();
					CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str());
				}
			}
			std::stringstream emsg;
			emsg << "Failed to present frame: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str());
		}
	}
}
