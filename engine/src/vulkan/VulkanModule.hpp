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

#include "vulkan/vulkan.hpp"// IWYU pragma: export
#include "vk_mem_alloc.hpp" // IWYU pragma: export
#include "glm/glm.hpp"		// IWYU pragma: export

#include <cstdint>
#include <set>
#include <utility>
#include <mutex>
#include <atomic>

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

	class TransientCommandContext {
	  public:
		vk::CommandPool pool;
		vk::Fence fence;

		static TransientCommandContext* Get();
		static void Cleanup();

		TransientCommandContext(const TransientCommandContext&) = delete;
		TransientCommandContext& operator=(const TransientCommandContext&) = delete;
		TransientCommandContext(TransientCommandContext&& o)
		  : pool(std::exchange(o.pool, {})), fence(std::exchange(o.fence, {})) {}
		TransientCommandContext& operator=(TransientCommandContext&& o) {
			pool = std::exchange(o.pool, {});
			fence = std::exchange(o.fence, {});
			return *this;
		}

	  private:
		static std::set<TransientCommandContext*> contexts;
		bool ready = false;

		TransientCommandContext() {}

		friend class VulkanCommandBuffer;
		friend class VulkanModule;
	};

	class RenderCommandContext {
	  public:
		vk::Semaphore acquire, render;
		vk::Fence fence;
		uint32_t imageIndex = UINT32_MAX;

		RenderCommandContext() {}
		RenderCommandContext(const RenderCommandContext&) = delete;
		RenderCommandContext& operator=(const RenderCommandContext&) = delete;
		RenderCommandContext(RenderCommandContext&& o)
		  : acquire(std::exchange(o.acquire, {})), render(std::exchange(o.render, {})), imageIndex(std::exchange(o.imageIndex, UINT32_MAX)) {}
		RenderCommandContext& operator=(RenderCommandContext&& o) {
			acquire = std::exchange(o.acquire, {});
			render = std::exchange(o.render, {});
			fence = std::exchange(o.fence, {});
			imageIndex = std::exchange(o.imageIndex, UINT32_MAX);
			return *this;
		}
	};

	//THIS IS NOT THREAD-SAFE
	class VulkanCommandBuffer : public CommandBuffer {
	  public:
		VulkanCommandBuffer() {}
		~VulkanCommandBuffer();
		VulkanCommandBuffer(VulkanCommandBuffer&&);
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&&);

		void Execute() override;

		vk::Fence GetFence();

		vk::CommandBuffer& vk();

	  protected:
		TransientCommandContext* transient = nullptr;
		RenderCommandContext* render = nullptr;
		bool didStartRender = false;

		vk::CommandPool* poolPtr = nullptr;
		std::promise<void> promise;
		vk::CommandBuffer primary;
		std::vector<vk::CommandBuffer> secondaries;

		bool SetupContext(bool rendering) override;
		void StartRendering(glm::vec3 clearColor) override;
		void EndRendering() override;

		friend class VulkanGPU;
		friend class VulkanModule;
	};

	class VulkanGPU final : public GPUManager::Impl {
	  public:
		std::shared_future<void> SubmitCmdBuffer(std::unique_ptr<CommandBuffer>&& cmd) override;
		void RunloopStart() override {}
		void RunloopStop() override;
		void RunloopIteration() override;

		bool IsRegenerating() override;

	  private:
		std::vector<std::unique_ptr<VulkanCommandBuffer>> submitted;
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
		Mesh::Impl* ConfigureMesh() override;
		Tex2D::Impl* ConfigureTex2D() override;
		Cubemap::Impl* ConfigureCubemap() override;
		GPUManager::Impl* ConfigureGPUManager() override;
		std::unique_ptr<CommandBuffer> CreateCmdBuffer() override;

		//==================== CORE VULKAN OBJECTS ====================
		vk::Instance instance;
		vk::PhysicalDevice physDev;
		vk::Device dev;
		vk::Queue queue;
		vma::Allocator allocator;

		//==================== PRESENTATION SUPPORT ====================
		vk::SurfaceKHR surface;
		vk::SurfaceFormatKHR surfaceFormat;
		struct SwapchainData {
			vk::SwapchainKHR chain;
			vk::Extent2D extent;
			std::vector<vk::Image> images;
			std::vector<vk::ImageView> views;
			std::vector<std::unique_ptr<RenderCommandContext>> renderContexts;
			uint16_t cycle = 0;
			std::atomic_bool regenRequested;
			std::atomic_bool regenInProgress;
		} swapchain;
		vk::CommandPool renderingPool;

		//==================== SECONDARY COMMAND BUFFER SUPPORT ====================
		vk::CommandBufferInheritanceRenderingInfo cbRenderingInheritance;
		vk::CommandBufferInheritanceInfo cbInheritance;

		//==================== DEPTH BUFFER ====================
		ViewImage depth;
		vk::Format selectedDF;

		//==================== GLOBALS UBO AND MEMORY ====================
		Allocated<vk::Buffer> globalsUBO;
		void* globalsMem;

		//==================== MISCELLANEOUS FIELDS ====================
		bool vsync;
		std::mutex queueMtx;

		VulkanModule()
		  : PALModule("vulkan") {
			swapchain.regenRequested.store(false);
			swapchain.regenInProgress.store(false);
		}
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