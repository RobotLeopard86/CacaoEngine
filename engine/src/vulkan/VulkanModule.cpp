#include "VulkanModule.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/PAL.hpp"
#include "ImplAccessor.hpp"

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
		GenSwapchain();
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
		if(auto vv = std::getenv("CACAO_DISABLE_VULKAN_VALIDATION"); vv == nullptr || (vv != nullptr && std::string(vv).compare("YES") != 0)) layers.push_back("VK_LAYER_KHRONOS_validation");
		if(auto ad = std::getenv("CACAO_ENABLE_APIDUMP"); ad != nullptr && std::string(ad).compare("YES") == 0) layers.push_back("VK_LAYER_LUNARG_api_dump");
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
			VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};
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
		Check<ExternalException>(physDev, "No devices support the required Vulkan extensions!", [this]() { instance.destroy(); });
		Logger::Engine(Logger::Level::Trace) << "Selected Vulkan device \"" << physDev.getProperties().deviceName << "\".";

		//Create logical device
		float priority = 1.0f;
		vk::DeviceQueueCreateInfo queueCI({}, 0, 1, &priority);
		vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures(VK_TRUE);
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures(VK_TRUE, &dynamicRenderingFeatures);
		vk::PhysicalDeviceSynchronization2Features sync2Features(VK_TRUE, &extendedDynamicStateFeatures);
		vk::PhysicalDeviceFeatures2 deviceFeatures2({}, &sync2Features);
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
		queue = dev.getQueue(0, 0);

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

		//Create globals UBO
		vk::BufferCreateInfo globalsCI({}, sizeof(glm::mat4) * 2, vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo globalsAllocCI({}, vma::MemoryUsage::eCpuToGpu);
		try {
			globalsUBO = allocator.createBuffer(globalsCI, globalsAllocCI);
		} catch(vk::SystemError& err) {
			TransientCommandContext::Cleanup();
			allocator.destroy();
			dev.destroy();
			instance.destroy();
			std::stringstream emsg;
			emsg << "Could not create globals uniform buffer: " << err.what();
			Check<ExternalException>(false, emsg.str());
		}

		//Map globals UBO
		if(allocator.mapMemory(globalsUBO.alloc, &globalsMem) != vk::Result::eSuccess) {
			allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
			TransientCommandContext::Cleanup();
			allocator.destroy();
			dev.destroy();
			instance.destroy();
			Check<ExternalException>(false, "Could not map globals uniform buffer memory!");
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
			TransientCommandContext::Cleanup();
			allocator.destroy();
			dev.destroy();
			instance.destroy();
			Check<ExternalException>(false, "Could not find any valid depth formats!");
		}

		didInit = true;
	}

	void VulkanModule::Term() {
		didInit = false;

		//Wait for the device to be idle so it's safe to destroy things
		dev.waitIdle();

		//Clean up transient command context objects
		TransientCommandContext::Cleanup();

		//Destroy Vulkan objects
		allocator.unmapMemory(globalsUBO.alloc);
		allocator.destroyBuffer(globalsUBO.obj, globalsUBO.alloc);
		dev.destroyImageView(depth.view);
		allocator.destroyImage(depth.obj, depth.alloc);
		allocator.destroy();
		dev.destroy();
		instance.destroy();
	}

	void VulkanModule::Disconnect() {
		connected = false;

		//Unsubscribe swapchain regeneration consumer
		EventManager::Get().UnsubscribeConsumer("WindowResize", resizer);

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