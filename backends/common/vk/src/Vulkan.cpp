#define VMA_IMPLEMENTATION
#include "VulkanCoreObjects.hpp"

#include "VkHooks.hpp"
#include "Graphics/Rendering/RenderController.hpp"
#include "Core/Assert.hpp"
#include "Utilities/MultiFuture.hpp"
#include "ExceptionCodes.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Cacao {
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
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
			VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
			VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
			VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME};
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

		//Create logical device
		float queuePriorities[2] = {1.0f, 2.0f};
		vk::DeviceQueueCreateInfo queueCI({}, 0, 2, queuePriorities);
		vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures(VK_TRUE);
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures(VK_TRUE, &dynamicRenderingFeatures);
		vk::PhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features(VK_TRUE, VK_TRUE, VK_TRUE, &extendedDynamicStateFeatures);
		vk::PhysicalDeviceSynchronization2Features sync2Features(VK_TRUE, &extendedDynamicState2Features);
		vk::PhysicalDeviceRobustness2FeaturesEXT robustnessFeatures(VK_TRUE, VK_TRUE, VK_TRUE, &sync2Features);
		vk::PhysicalDeviceFeatures2 deviceFeatures2 {};
		deviceFeatures2.pNext = &robustnessFeatures;
		vk::DeviceCreateInfo deviceCI({}, queueCI, {}, requiredDevExts, nullptr, &deviceFeatures2);
		try {
			dev = physDev.createDevice(deviceCI);
		} catch(vk::SystemError& err) {
			vk_instance.destroy();
			EngineAssert(false, "The logical device could not be created!");
		}

		//Get queues
		graphicsQueue = dev.getQueue(0, 0);
		immediateQueue = dev.getQueue(0, 1);

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

		//Create command pools
		vk::CommandPoolCreateInfo rpoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 0);
		vk::CommandPoolCreateInfo ipoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, 0);
		try {
			renderPool = dev.createCommandPool(rpoolCI);
		} catch(vk::SystemError& err) {
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			EngineAssert(false, "Could not create rendering command pool!");
		}
		try {
			immediatePool = dev.createCommandPool(ipoolCI);
		} catch(vk::SystemError& err) {
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			EngineAssert(false, "Could not create immediate command pool!");
		}

		//Allocate immediate command buffers
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
		vk::CommandBufferAllocateInfo allocCI(immediatePool, vk::CommandBufferLevel::ePrimary, poolThreads.size());
		std::vector<vk::CommandBuffer> immBufs;
		try {
			immBufs = dev.allocateCommandBuffers(allocCI);
		} catch(vk::SystemError& err) {
			dev.destroyCommandPool(immediatePool);
			dev.destroyCommandPool(renderPool);
			allocator.destroy();
			dev.destroy();
			vk_instance.destroy();
			EngineAssert(false, "Could not allocate immediate command buffers!");
		}
		for(int i = 0; i < immBufs.size(); i++) {
			Immediate imm;
			imm.cmd = immBufs[i];
			try {
				imm.fence = dev.createFence({vk::FenceCreateFlagBits::eSignaled});
			} catch(vk::SystemError& err) {
				for(; i >= 0; i--) {
					dev.destroyFence(immediates[poolThreads[i]].fence);
					immediates.erase(poolThreads[i]);
				}
				dev.freeCommandBuffers(immediatePool, immBufs);
				dev.destroyCommandPool(immediatePool);
				dev.destroyCommandPool(renderPool);
				allocator.destroy();
				dev.destroy();
				vk_instance.destroy();
				EngineAssert(false, "Could not create immediate fences!");
			}
			immediates.insert_or_assign(poolThreads[i], imm);
		}
	}

	void RegisterGraphicsExceptions() {
		Exception::RegisterExceptionCode(100, "Vulkan");
		Exception::RegisterExceptionCode(101, "WaitExpired");
	}

	void RenderController::UpdateGraphicsState() {}

	void RenderController::Shutdown() {
		//Destroy Vulkan objects
		std::vector<vk::CommandBuffer> cbufs;
		for(auto i : immediates) {
			dev.destroyFence(i.second.fence);
			cbufs.push_back(i.second.cmd);
		}
		dev.freeCommandBuffers(immediatePool, cbufs);
		cbufs.clear();
		for(auto f : frames) {
			dev.destroySemaphore(f.acquireSemaphore);
			dev.destroySemaphore(f.renderSemaphore);
			dev.destroyFence(f.fence);
			cbufs.push_back(f.cmd);
		}
		dev.freeCommandBuffers(renderPool, cbufs);
		dev.destroyCommandPool(immediatePool);
		dev.destroyCommandPool(renderPool);
		allocator.destroy();
		dev.destroy();
		vk_instance.destroy();
	}
}