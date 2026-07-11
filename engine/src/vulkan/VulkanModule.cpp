#include "VulkanModule.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/PAL.hpp"
#include "ImplAccessor.hpp"
#include "SafeGetenv.hpp"

#ifdef __linux__
#include "impl/Window.hpp"
#endif

#include <memory>

namespace Cacao {
	struct VulkanModuleRegistrar {
		VulkanModuleRegistrar() {
			IMPL(PAL).registry.insert_or_assign("vulkan", []() { vulkan = std::make_shared<VulkanModule>(); return vulkan; });
		}
	};
	__attribute__((used)) VulkanModuleRegistrar vkmr;

	void VulkanModule::Destroy() {
		vulkan.reset();
	}

	void VulkanModule::SetVSync(bool state) {
		vsync = state;
		Event e("INTERNAL-RegenSwapchain");
		EventManager::Get().Dispatch(e);
	}

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
		std::vector<vk::PhysicalDevice> devWork = *devices;
		std::sort(devWork.begin(), devWork.end(), [&scores, devices](const vk::PhysicalDevice& a, const vk::PhysicalDevice& b) {
			auto indexA = std::distance(devices->begin(), std::find(devices->begin(), devices->end(), a));
			auto indexB = std::distance(devices->begin(), std::find(devices->begin(), devices->end(), b));
			return scores[indexA] > scores[indexB];
		});

		*devices = devWork;
	}

	void VulkanModule::Init() {
		Check<BadInitStateException>(!didInit, "The Vulkan module must be uninitialized when Init is called!");
		didInit = true;

		//Initialize base Vulkan functions
		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		//Get required layers
		std::vector<const char*> layers = {};
#ifdef _DEBUG
		//if(safe_getenv("CACAO_DISABLE_VULKAN_VALIDATION").compare("YES") != 0) layers.push_back("VK_LAYER_KHRONOS_validation");
		if(safe_getenv("CACAO_ENABLE_VULKAN_API_DUMP").compare("YES") == 0) layers.push_back("VK_LAYER_LUNARG_api_dump");
#endif

		//Create instance
		vk::ApplicationInfo appInfo("Cacao Engine Vulkan Module", 1, "Cacao Engine", 1, VK_API_VERSION_1_3);
		std::vector<const char*> requiredInstanceExts;
		requiredInstanceExts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef __linux__
		if(IMPL(Window).ProviderID().compare("x11") == 0)
			requiredInstanceExts.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
		else
			requiredInstanceExts.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#ifdef _WIN32
		requiredInstanceExts.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
		vk::InstanceCreateInfo instanceCI({}, &appInfo, layers, requiredInstanceExts);
		try {
			instance = vk::createInstance(instanceCI);
		} catch(const vk::SystemError& err) {
			Check<ExternalException>(false, std::string("Vulkan instance could not be created: ") + err.what());
		}

		//Initialize instance Vulkan functions
		VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

		//Find best device
		std::vector<const char*> requiredDevExts = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
			VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME};
#ifdef __linux__
		requiredDevExts.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
#endif
		auto physicalDevices = instance.enumeratePhysicalDevices();
		Check<ExternalException>(!physicalDevices.empty(), "There are no Vulkan-compatible devices!", [this]() { instance.destroy(); });
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
		Check<ExternalException>(!okDevs.empty(), "No devices support the required Vulkan extensions!", [this]() { instance.destroy(); });
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
		Logger::Engine(Logger::Level::Trace) << "Selected Vulkan device \"" << physDev.getProperties().deviceName << "\".";

		//Select queue family
		std::vector<vk::QueueFamilyProperties2> queueProps = physDev.getQueueFamilyProperties2();
		int queueFamily = -1;
		int counter = -1;
		for(const vk::QueueFamilyProperties2& props : queueProps) {
			++counter;
			if(props.queueFamilyProperties.queueCount < 1) continue;
			if(!(props.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)) continue;
			if(!(props.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer)) continue;
			queueFamily = counter;
			break;
		}
		Check<ExternalException>(queueFamily >= 0, "No queues support the required Vulkan operations!");

		//Create logical device
		float priority = 1.0f;
		vk::DeviceQueueCreateInfo queueCI({}, queueFamily, 1, &priority);
		vk::PhysicalDeviceVulkan13Features vulkan13Features {};
		vulkan13Features.setDynamicRendering(VK_TRUE);
		vulkan13Features.setSynchronization2(VK_TRUE);
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures(VK_TRUE, &vulkan13Features);
		vk::PhysicalDeviceVulkan12Features vulkan12Features {};
		vulkan12Features.setTimelineSemaphore(VK_TRUE);
		vulkan12Features.setPNext(&extendedDynamicStateFeatures);
		vk::PhysicalDeviceFeatures2 deviceFeatures2({}, &vulkan12Features);
		deviceFeatures2.features.setRobustBufferAccess(VK_TRUE);
		deviceFeatures2.features.setIndependentBlend(VK_TRUE);
		deviceFeatures2.features.setOcclusionQueryPrecise(VK_TRUE);
		deviceFeatures2.features.setPipelineStatisticsQuery(VK_TRUE);
		vk::DeviceCreateInfo deviceCI({}, queueCI, {}, requiredDevExts, nullptr, &deviceFeatures2);
		try {
			dev = physDev.createDevice(deviceCI);
		} catch(vk::SystemError& err) {
			instance.destroy();
			Check<ExternalException>(false, "The logical device could not be created!");
		}

		//Initialize device Vulkan functions
		VULKAN_HPP_DEFAULT_DISPATCHER.init(instance, dev);

		//Get queue
		queue = dev.getQueue(queueFamily, 0);

		//Create memory allocator
		vma::VulkanFunctions vkFuncs(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr, VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr);
		vma::AllocatorCreateInfo allocatorCI({}, physDev, dev, 0UL, 0, 0, 0, &vkFuncs, instance, VK_API_VERSION_1_3);
		try {
			allocator = vma::createAllocator(allocatorCI);
		} catch(vk::SystemError& err) {
			dev.destroy();
			instance.destroy();
			Check<ExternalException>(false, "Could not create memory allocator!");
		}

		//Make transient command context for main thread
		TransientCommandContext::Get();

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
			TransientCommandContext::Cleanup();
			allocator.destroy();
			dev.destroy();
			instance.destroy();
			Check<ExternalException>(false, "Could not find any valid depth formats!");
		}

		//Create rendering command pool
		vk::CommandPoolCreateInfo renderPoolCI({}, 0);
		try {
			renderingPool = vulkan->dev.createCommandPool(renderPoolCI);
		} catch(vk::SystemError& err) {
			selectedDF = vk::Format::eUndefined;
			TransientCommandContext::Cleanup();
			allocator.destroy();
			dev.destroy();
			instance.destroy();
			Check<ExternalException>(false, "Could not create rendering command pool!");
		}

		//Create engine descriptor set layout
		std::vector<vk::DescriptorSetLayoutBinding> dsBindings;
		dsBindings.emplace_back(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, VK_NULL_HANDLE);
		dsBindings.emplace_back(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, VK_NULL_HANDLE);
		try {
			engineSetLayout = vulkan->dev.createDescriptorSetLayout({{}, dsBindings});
		} catch(vk::SystemError& err) {
			vulkan->dev.destroyCommandPool(renderingPool);
			selectedDF = vk::Format::eUndefined;
			TransientCommandContext::Cleanup();
			allocator.destroy();
			dev.destroy();
			instance.destroy();
			Check<ExternalException>(false, "Could not create rendering command pool!");
		}

		didInit = true;
	}

	void VulkanModule::Term() {
		didInit = false;

		//Wait for the device to be idle so it's safe to destroy things
		dev.waitIdle();

		//Clean up descriptor pool
		if(descriptorPool) {
			dev.resetDescriptorPool(descriptorPool);
			dev.destroyDescriptorPool(descriptorPool);
		}

		//Clean up rendering context objects
		for(RenderCommandContext& rc : swapchain.contexts) {
			if(rc.rendered) vulkan->dev.destroySemaphore(rc.rendered);
			if(rc.inFlight) vulkan->dev.destroyFence(rc.inFlight);
			if(rc.sync.semaphore) vulkan->dev.destroySemaphore(rc.sync.semaphore);
			if(rc.globals.obj) {
				allocator.unmapMemory(rc.globals.alloc);
				allocator.destroyBuffer(rc.globals.obj, rc.globals.alloc);
			}
			if(rc.camData.obj) {
				allocator.unmapMemory(rc.camData.alloc);
				allocator.destroyBuffer(rc.camData.obj, rc.camData.alloc);
			}
		}
		for(vk::Semaphore& sem : swapchain.acquireSems) {
			if(sem) vulkan->dev.destroySemaphore(sem);
		}

		//Clean up descriptor set layout
		vulkan->dev.destroyDescriptorSetLayout(engineSetLayout);

		//Clean up transient command context objects
		TransientCommandContext::Cleanup();

		//Destroy depth images
		for(std::size_t i = 0; i < vulkan->swapchain.depthImages.size(); ++i) {
			//Get slot reference
			ViewImage& vi = vulkan->swapchain.depthImages[i];

			//Destroy old objects
			vulkan->dev.destroyImageView(vi.view);
			vulkan->allocator.destroyImage(vi.obj, vi.alloc);
		}

		//Destroy Vulkan objects
		dev.destroyCommandPool(renderingPool);
		allocator.destroy();
		dev.destroy();
		instance.destroy();
	}

	void VulkanModule::Disconnect() {
		connected = false;

		//Destroy swapchain
		for(const vk::ImageView& view : swapchain.views) {
			dev.destroyImageView(view);
		}
		dev.destroySwapchainKHR(swapchain.chain);

		//Destroy surface
		instance.destroySurfaceKHR(surface);
	}

	std::unique_ptr<CommandBuffer> VulkanModule::CreateCmdBuffer() {
		return std::make_unique<VulkanCommandBuffer>();
	}
}