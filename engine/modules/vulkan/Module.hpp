#pragma once

#include "PALCommon.hpp"

#ifdef __linux__
#ifdef HAS_X11
#define VK_USE_PLATFORM_XCB_KHR
#endif
#ifdef HAS_WAYLAND
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#endif
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.hpp"
#include "glm/glm.hpp"

#include <vector>
#include <thread>
#include <mutex>

namespace Cacao {
	template<typename T>
	struct Allocated {
	  public:
		vma::Allocation alloc;
		T obj;
	};

	struct ViewImage : public Allocated<vk::Image> {
		vk::ImageView view;
	};

	class Immediate {
	  public:
		vk::CommandPool pool;
		vk::CommandBuffer cmd;
		vk::Fence fence;

		static Immediate Get();
		static void Cleanup();

		void Submit(bool wait);

	  private:
		static std::map<std::thread::id, Immediate> immediates;
	};

	class VulkanModule : public PALModule {
	  public:
		void Init() override;
		void Term() override;
		void Connect() override;
		void Disconnect() override;
		void Destroy() override;

		/* ------------------------------------------- *\
		|*      PLACEHOLDER: IMPL CONFIGURATORS        *|
		\* ------------------------------------------- */

		vk::Instance instance;
		vk::PhysicalDevice physDev;
		vk::Device dev;
		vk::SurfaceKHR surface;
		vk::SurfaceFormatKHR surfaceFormat;
		struct Swapchain {
			vk::SwapchainKHR chain;
			std::vector<vk::Image> images;
			std::vector<vk::ImageView> views;
		} swapchain;
		vma::Allocator allocator;
		ViewImage depth;
		vk::Format selectedDF;
		vk::Queue gfxQueue;
		vk::CommandPool renderPool;
		Allocated<vk::Buffer> globalsUBO;
		void* globalsMem;

		VulkanModule()
		  : PALModule("vulkan") {}
		~VulkanModule() {}

	  private:
		vk::Queue immQueue;
		std::mutex immqLock;
		friend Immediate;
	};

	inline std::shared_ptr<VulkanModule> vulkan;

	void GenSwapchain();

	inline vk::PresentModeKHR presentMode;

	constexpr glm::mat4 projectionCorrection(
		{1.0f, 0.0f, 0.0f, 0.0f}, //No X change
		{0.0f, -1.0f, 0.0f, 0.0f},//Invert Y
		{0.0f, 0.0f, 0.5f, 0.0f}, //Halve depth range: [-1, 1] (OpenGL range) -> [-0.5, 0.5]
		{0.0f, 0.0f, 0.5f, 1.0f});//Shift depth range: [-0.5, 0.5] => [0, 1] (Vulkan range)
}