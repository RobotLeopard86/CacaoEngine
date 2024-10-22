#define VMA_IMPLEMENTATION
#include "VulkanCoreObjects.hpp"

#include "VkHooks.hpp"
#include "ActiveItems.hpp"
#include "Graphics/Rendering/RenderController.hpp"
#include "Core/Assert.hpp"
#include "Utilities/MultiFuture.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "ExceptionCodes.hpp"
#include "UIViewShaderManager.hpp"
#include "UI/Shaders.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

bool backendInitBeforeWindow = true;

namespace Cacao {
	Allocated<vk::Buffer> uiQuadBuffer;

	void RenderController::Init() {
		CheckException(!isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot initialize the initialized render controller!")
		isInitialized = true;

		//Initialize base Vulkan functions
		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		//Create instance
		std::vector<const char*> layers = {};
#ifdef DEBUG
		layers.push_back("VK_LAYER_KHRONOS_validation");
		if(auto ad = std::getenv("ENABLE_APIDUMP"); ad != nullptr && std::string(ad).compare("YES") == 0) layers.push_back("VK_LAYER_LUNARG_api_dump");
#endif
		vk::ApplicationInfo appInfo("Cacao Engine Client", 1, "Cacao Engine", 1, VK_API_VERSION_1_2);
		auto requiredInstanceExts = GetRequiredInstanceExts();
#ifdef __APPLE__
		requiredInstanceExts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
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
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
			VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME,
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
			VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
			VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
			VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
			VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
			VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};
#ifdef __linux__
		requiredDevExts.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
#endif
		auto physicalDevices = vk_instance.enumeratePhysicalDevices();
		EngineAssert(!physicalDevices.empty(), "There are no Vulkan-compatible devices!");
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
				std::this_thread::sleep_for(std::chrono::milliseconds(40));
			}
			if(!good) break;
			physDev = pdev;
		}
		if(!physDev) {
			vk_instance.destroy();
			EngineAssert(false, "No devices support the required Vulkan extensions!");
		}

		//Get pool threads
		MultiFuture<std::thread::id> poolThreadsFut;
		for(int i = 0; i < Engine::GetInstance()->GetThreadPool()->size(); i++) {
			poolThreadsFut.push_back(Engine::GetInstance()->GetThreadPool()->enqueue([]() {
				return std::this_thread::get_id();
			}));
		}
		poolThreadsFut.WaitAll();
		std::vector<std::thread::id> poolThreads;
		for(int i = 0; i < poolThreadsFut.size(); i++) {
			poolThreads.push_back(poolThreadsFut[i].get());
		}
		poolThreads.push_back(std::this_thread::get_id());

		//Create logical device
		std::vector<float> queuePriorities = {0.5f};
		for(int i = 0; i < poolThreads.size(); i++) {
			queuePriorities.push_back(1.0f);
		}
		vk::DeviceQueueCreateInfo queueCI({}, 0, queuePriorities);
		vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures(VK_TRUE);
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures(VK_TRUE, &dynamicRenderingFeatures);
		vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT extendedDynamicState3Features {};
		extendedDynamicState3Features.extendedDynamicState3ColorBlendEnable = VK_TRUE;
		extendedDynamicState3Features.extendedDynamicState3ColorBlendEquation = VK_TRUE;
		extendedDynamicState3Features.pNext = &extendedDynamicStateFeatures;
		vk::PhysicalDeviceSynchronization2Features sync2Features(VK_TRUE, &extendedDynamicState3Features);
		vk::PhysicalDeviceRobustness2FeaturesEXT robustnessFeatures(VK_FALSE, VK_FALSE, VK_TRUE, &sync2Features);
		vk::PhysicalDeviceFeatures2 deviceFeatures2 {};
		deviceFeatures2.pNext = &robustnessFeatures;
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
		graphicsQueue = dev.getQueue(0, 0);

		//Create memory allocator
		vma::VulkanFunctions vkFuncs(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr, VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr);
		vma::AllocatorCreateInfo allocatorCI({}, physDev, dev, 0UL, 0, 0, 0, &vkFuncs, vk_instance, VK_API_VERSION_1_2);
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

		vk::CommandPoolCreateInfo ipoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, 0);
		for(int i = 0; i < poolThreads.size(); i++) {
			Immediate imm;
			imm.queue = dev.getQueue(0, i + 1);
			try {
				imm.pool = dev.createCommandPool(ipoolCI);
			} catch(vk::SystemError& err) {
				for(; i >= 0; i--) {
					dev.destroyCommandPool(immediates[poolThreads[i]].pool);
					immediates.erase(poolThreads[i]);
				}
				dev.destroyCommandPool(renderPool);
				allocator.destroy();
				dev.destroy();
				vk_instance.destroy();
				EngineAssert(false, "Could not create immediate pools!");
			}
			try {
				vk::CommandBufferAllocateInfo allocCI(imm.pool, vk::CommandBufferLevel::ePrimary, 1);
				imm.cmd = dev.allocateCommandBuffers(allocCI)[0];
			} catch(vk::SystemError& err) {
				for(; i >= 0; i--) {
					dev.freeCommandBuffers(immediates[poolThreads[i]].pool, immediates[poolThreads[i]].cmd);
					dev.destroyCommandPool(immediates[poolThreads[i]].pool);
					immediates.erase(poolThreads[i]);
				}
				dev.destroyCommandPool(renderPool);
				allocator.destroy();
				dev.destroy();
				vk_instance.destroy();
				EngineAssert(false, "Could not create immediate pools!");
			}
			try {
				imm.fence = dev.createFence({vk::FenceCreateFlagBits::eSignaled});
			} catch(vk::SystemError& err) {
				for(; i >= 0; i--) {
					dev.freeCommandBuffers(immediates[poolThreads[i]].pool, immediates[poolThreads[i]].cmd);
					dev.destroyCommandPool(immediates[poolThreads[i]].pool);
					dev.destroyFence(immediates[poolThreads[i]].fence);
					immediates.erase(poolThreads[i]);
				}
				dev.destroyCommandPool(renderPool);
				allocator.destroy();
				dev.destroy();
				vk_instance.destroy();
				EngineAssert(false, "Could not create immediate fences!");
			}
			immediates.insert_or_assign(poolThreads[i], imm);
		}

		//Create globals UBO
		vk::BufferCreateInfo globalsCI({}, sizeof(glm::mat4) * 2, vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo globalsAllocCI({}, vma::MemoryUsage::eCpuToGpu);
		try {
			auto [globals, alloc] = allocator.createBuffer(globalsCI, globalsAllocCI);
			globalsUBO = {.alloc = alloc, .obj = globals};
		} catch(vk::SystemError& err) {
			for(int i = 0; i < immediates.size(); i++) {
				dev.freeCommandBuffers(immediates[poolThreads[i]].pool, immediates[poolThreads[i]].cmd);
				dev.destroyCommandPool(immediates[poolThreads[i]].pool);
				dev.destroyFence(immediates[poolThreads[i]].fence);
				immediates.erase(poolThreads[i]);
			}
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
			for(int i = 0; i < immediates.size(); i++) {
				dev.freeCommandBuffers(immediates[poolThreads[i]].pool, immediates[poolThreads[i]].cmd);
				dev.destroyCommandPool(immediates[poolThreads[i]].pool);
				dev.destroyFence(immediates[poolThreads[i]].fence);
				immediates.erase(poolThreads[i]);
			}
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			EngineAssert(false, "Could not map globals uniform buffer memory!");
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
			allocator.unmapMemory(globalsUBO.alloc);
			allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
			for(int i = 0; i < immediates.size(); i++) {
				dev.freeCommandBuffers(immediates[poolThreads[i]].pool, immediates[poolThreads[i]].cmd);
				dev.destroyCommandPool(immediates[poolThreads[i]].pool);
				dev.destroyFence(immediates[poolThreads[i]].fence);
				immediates.erase(poolThreads[i]);
			}
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
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f};
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
		Immediate imm = immediates[std::this_thread::get_id()];
		vk::CommandBufferBeginInfo copyBegin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		imm.cmd.begin(copyBegin);
		{
			vk::BufferCopy2 copy(0UL, 0UL, vbsz);
			vk::CopyBufferInfo2 copyInfo(vertexUp.obj, uiQuadBuffer.obj, copy);
			imm.cmd.copyBuffer2(copyInfo);
		}
		imm.cmd.end();
		if(dev.getFenceStatus(imm.fence) == vk::Result::eSuccess) {
			vk::Result fenceWait = dev.waitForFences(imm.fence, VK_TRUE, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1000)).count());
			CheckException(fenceWait == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("WaitExpired"), "Waited too long for immediate fence reset!")
			dev.resetFences(imm.fence);
		}
		vk::CommandBufferSubmitInfo cbsi(imm.cmd);
		vk::SubmitInfo2 si({}, {}, cbsi);
		imm.queue.submit2(si, imm.fence);
		dev.waitForFences(imm.fence, VK_TRUE, INFINITY);
		allocator.destroyBuffer(vertexUp.obj, vertexUp.alloc);

		didGenShaders = false;

		frameCycle = 0;
	}

	void RegisterGraphicsExceptions() {
		Exception::RegisterExceptionCode(100, "Vulkan");
		Exception::RegisterExceptionCode(101, "WaitExpired");
	}

	void RenderController::UpdateGraphicsState() {}

	void RenderController::WaitGPUIdleBeforeTerminate() {
		dev.waitIdle();
	}

	void RenderController::Shutdown() {
		//Wait for the device to be idle
		dev.waitIdle();

		if(didGenShaders) {
			//Cleanup UI shaders
			DelShaders();
			uivsm.Release();
			didGenShaders = false;
		}

		//Destroy Vulkan objects
		allocator.unmapMemory(globalsUBO.alloc);
		allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
		std::vector<vk::CommandBuffer> cbufs;
		for(auto imm : immediates) {
			dev.freeCommandBuffers(imm.second.pool, imm.second.cmd);
			dev.destroyCommandPool(imm.second.pool);
			dev.destroyFence(imm.second.fence);
		}
		immediates.clear();
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
					CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str())
				}
			}
			std::stringstream emsg;
			emsg << "Failed to acquire swapchain image: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str())
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
			glm::ivec2 winSize = Window::GetInstance()->GetContentAreaSize();
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
		f.cmd.setColorBlendEnableEXT(0, VK_FALSE);
		f.cmd.setColorBlendEquationEXT(0, vk::ColorBlendEquationEXT(vk::BlendFactor::eZero, vk::BlendFactor::eOne, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eOne, vk::BlendOp::eAdd));
		f.cmd.setDepthTestEnable(VK_TRUE);
		f.cmd.setDepthCompareOp(vk::CompareOp::eLess);

		//Start rendering
		constexpr glm::vec3 clearColorSRGB {float(0xCF) / 256, 1.0f, float(0x4D) / 256};
		glm::vec3 clearColorLinear = glm::pow(clearColorSRGB, glm::vec3 {2.2f});
		vk::RenderingAttachmentInfo colorAttachment(imageViews[imgIdx], vk::ImageLayout::eColorAttachmentOptimal, {}, {}, {}, vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore, vk::ClearColorValue(std::array<float, 4> {clearColorLinear.r, clearColorLinear.g, clearColorLinear.b, 1.0f}));
		vk::RenderingAttachmentInfo depthAttachment(depthView, vk::ImageLayout::eDepthAttachmentOptimal, {}, {}, {}, vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare, vk::ClearDepthStencilValue(1.0f, 0.0f));
		vk::RenderingInfo renderingInfo({}, vk::Rect2D({0, 0}, extent), 1, 0, colorAttachment, &depthAttachment);
		f.cmd.beginRendering(renderingInfo);

		//Draw scene
		for(RenderObject& obj : frame->objects) {
			//Bind shader
			obj.material.shader->Bind();

			//Upload material data to shader
			obj.material.shader->UploadData(obj.material.data, obj.transformMatrix);

			//Draw the mesh
			obj.mesh->Draw();

			//Unbind any textures
			const ShaderSpec& spec = obj.material.shader->GetSpec();
			for(ShaderUploadItem& sui : obj.material.data) {
				if(std::find_if(spec.begin(), spec.end(), [&sui](ShaderItemInfo sii) {
					   return (sii.type == SpvType::SampledImage && sii.entryName == sui.target);
				   }) != spec.end()) {
					if(sui.data.type() == typeid(Texture2D*)) {
						Texture2D* tex = std::any_cast<Texture2D*>(sui.data);
						tex->Unbind();
					} else if(sui.data.type() == typeid(Cubemap*)) {
						Cubemap* tex = std::any_cast<Cubemap*>(sui.data);
						tex->Unbind();
					} else if(sui.data.type() == typeid(UIView*)) {
						UIView* view = std::any_cast<UIView*>(sui.data);
						view->Unbind();
					} else if(sui.data.type() == typeid(AssetHandle<Texture2D>)) {
						AssetHandle<Texture2D> tex = std::any_cast<AssetHandle<Texture2D>>(sui.data);
						tex->Unbind();
					} else if(sui.data.type() == typeid(AssetHandle<Cubemap>)) {
						AssetHandle<Cubemap> tex = std::any_cast<AssetHandle<Cubemap>>(sui.data);
						tex->Unbind();
					} else if(sui.data.type() == typeid(AssetHandle<UIView>)) {
						AssetHandle<UIView> view = std::any_cast<AssetHandle<UIView>>(sui.data);
						view->Unbind();
					}
				}
			}

			//Unbind shader
			obj.material.shader->Unbind();
		}

		//Draw skybox (if one exists)
		if(!frame->skybox.IsNull()) frame->skybox->Draw(frame->projection, frame->view);

		//Draw UI if it's been rendered
		if(Engine::GetInstance()->GetGlobalUIView()->HasBeenRendered()) {
			//Create projection matrix
			glm::mat4 project = projectionCorrection * glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

			//Upload uniforms
			ShaderUploadData uiud;
			uiud.emplace_back(ShaderUploadItem {.target = "uiTex", .data = std::any(Engine::GetInstance()->GetGlobalUIView().get())});
			uivsm->Bind();
			uivsm->UploadData(uiud, glm::identity<glm::mat4>());
			Shader::UploadCacaoGlobals(project, glm::identity<glm::mat4>());//Kinda scary but it'll get overwritten for the next frame

			//Modify dynamic state
			f.cmd.setColorBlendEnableEXT(0, VK_TRUE);
			f.cmd.setColorBlendEquationEXT(0, vk::ColorBlendEquationEXT(vk::BlendFactor::eSrcColor, vk::BlendFactor::eOneMinusSrcColor, vk::BlendOp::eAdd, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha));
			f.cmd.setDepthTestEnable(VK_FALSE);
			f.cmd.setDepthCompareOp(vk::CompareOp::eAlways);

			//Draw quad
			constexpr std::array<vk::DeviceSize, 1> offsets = {{0}};
			f.cmd.bindVertexBuffers(0, uiQuadBuffer.obj, offsets);
			f.cmd.draw(18, 1, 0, 0);

			//Unbind UI shader and view
			Engine::GetInstance()->GetGlobalUIView()->Unbind();
			uivsm->Unbind();
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
		vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::SubmitInfo submitInfo(f.acquireSemaphore, waitStage, f.cmd, f.renderSemaphore);
		if(graphicsQueue.submit(1, &submitInfo, f.fence) != vk::Result::eSuccess) {
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), "Failed to submit frame command buffer!")
		}

		//Set submission for presenting
		submission = {.sem = f.renderSemaphore, .image = imgIdx};

		//Cycle the frame
		frameCycle = (frameCycle + 1) % frames.size();
	}
}