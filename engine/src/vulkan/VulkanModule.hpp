#pragma once

#include "Cacao/GPU.hpp"
#include "impl/PAL.hpp"
#include "Cacao/EventConsumer.hpp"

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

#include <map>
#include <thread>
#include <mutex>

namespace Cacao {
	template<typename T>
	struct Allocated {
	  public:
		vma::Allocation alloc;
		T obj;

		Allocated() {}
		Allocated(std::pair<T, vma::Allocation> p) : alloc(p.second), obj(p.first) {}
	};

	struct ViewImage : public Allocated<vk::Image> {
		vk::ImageView view;

		ViewImage() {}
		ViewImage(std::pair<vk::Image, vma::Allocation> p) : Allocated<vk::Image>(p) {}
	};

	class Immediate {
	  public:
		vk::CommandPool pool;
		vk::CommandBuffer cmd;
		vk::Fence fence;
		std::mutex mtx;

		static void Cleanup();

	  private:
		static std::map<std::thread::id, Immediate> immediates;

		static Immediate& Get();
		friend class VulkanCommandBuffer;
		friend class VulkanModule;
	};

	//THIS IS NOT THREAD-SAFE
	class VulkanCommandBuffer : public CommandBuffer {
	  public:
		VulkanCommandBuffer();
		VulkanCommandBuffer(VulkanCommandBuffer&&);
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&&);

		vk::CommandBuffer* operator->() {
			return &imm.get().cmd;
		}

		void Execute() override;

	  private:
		std::reference_wrapper<Immediate> imm;
		std::unique_lock<std::mutex> lock;
		std::promise<void> promise;
		friend class VulkanGPU;
	};

	class VulkanGPU final : public GPUManager::Impl {
	  public:
		std::shared_future<void> SubmitCmdBuffer(CommandBuffer&& cmd) override;
		void RunloopStart() override {}
		void RunloopStop() override {}
		void RunloopIteration() override;

	  private:
		std::vector<VulkanCommandBuffer> submitted;
		std::mutex mutex;
	};

	class VulkanModule final : public PALModule {
	  public:
		void Init() override;
		void Term() override;
		void Connect() override;
		void Disconnect() override;
		void Destroy() override;
		void SetVSync(bool state) override;

		//==================== IMPL POINTER CONFIGURATION ====================
		virtual Mesh::Impl* ConfigureMesh() override;
		virtual Tex2D::Impl* ConfigureTex2D() override;
		virtual Cubemap::Impl* ConfigureCubemap() override;
		virtual GPUManager::Impl* ConfigureGPUManager() override;

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
		vk::Queue queue;
		std::mutex queueMtx;
		vk::CommandPool renderPool;
		Allocated<vk::Buffer> globalsUBO;
		void* globalsMem;
		bool vsync;

		VulkanModule()
		  : PALModule("vulkan") {}
		~VulkanModule() {}

	  private:
		EventConsumer resizer;
	};

	inline std::shared_ptr<VulkanModule> vulkan;

	void GenSwapchain();

	constexpr glm::mat4 projectionCorrection(
		{1.0f, 0.0f, 0.0f, 0.0f}, //No X change
		{0.0f, -1.0f, 0.0f, 0.0f},//Invert Y
		{0.0f, 0.0f, 0.5f, 0.0f}, //Halve depth range: [-1, 1] (OpenGL range) -> [-0.5, 0.5]
		{0.0f, 0.0f, 0.5f, 1.0f});//Shift depth range: [-0.5, 0.5] => [0, 1] (Vulkan range)
}