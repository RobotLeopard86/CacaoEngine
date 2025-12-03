#pragma once

#include "Cacao/GPU.hpp"
#include "impl/PAL.hpp"
#include "Cacao/EventConsumer.hpp"
#include <semaphore>

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

	class VulkanCommandBuffer;

	class GfxHandler {
	  public:
		vk::Semaphore acquireImage;
		vk::Semaphore doneRendering;
		vk::Fence imageFence;
		unsigned int imageIdx;
		std::atomic_bool inUse;

		GfxHandler()
		  : imageIdx(UINT32_MAX), inUse(false) {}
		GfxHandler(const GfxHandler&) = delete;
		GfxHandler& operator=(const GfxHandler&) = delete;
		GfxHandler(GfxHandler&& o)
		  : acquireImage(std::exchange(o.acquireImage, {})), doneRendering(std::exchange(o.doneRendering, {})), imageFence(std::exchange(o.imageFence, {})), imageIdx(std::exchange(o.imageIdx, UINT32_MAX)), inUse(o.inUse.load(std::memory_order_relaxed)) {}
		GfxHandler& operator=(GfxHandler&& o) {
			acquireImage = std::exchange(o.acquireImage, {});
			doneRendering = std::exchange(o.doneRendering, {});
			imageFence = std::exchange(o.imageFence, {});
			imageIdx = std::exchange(o.imageIdx, UINT32_MAX);
			inUse.store(o.inUse.load(std::memory_order_relaxed));
			return *this;
		}

		void Acquire();
		void MakeDrawable(VulkanCommandBuffer*);
		void MakePresentable(VulkanCommandBuffer*);

		int id;

	  private:
		static std::vector<std::shared_ptr<GfxHandler>> handlers;

		friend class Immediate;
		friend class VulkanCommandBuffer;
		friend class VulkanGPU;
		friend class VulkanModule;
		friend void GenSwapchain();
	};

	class Immediate {
	  public:
		vk::CommandPool pool;
		vk::CommandBuffer cmd;
		vk::Fence fence;
		std::binary_semaphore accessMgr;
		std::shared_ptr<GfxHandler> gfx;

		static void Cleanup();

		Immediate(const Immediate&) = delete;
		Immediate& operator=(const Immediate&) = delete;
		Immediate(Immediate&& o)
		  : pool(std::exchange(o.pool, {})), cmd(std::exchange(o.cmd, {})), fence(std::exchange(o.fence, {})), accessMgr(1), gfx(std::exchange(o.gfx, {})) {}
		Immediate& operator=(Immediate&& o) {
			pool = std::exchange(o.pool, {});
			cmd = std::exchange(o.cmd, {});
			gfx = std::exchange(o.gfx, {});
			return *this;
		}

	  private:
		static std::set<Immediate*> imms;
		bool ready = false;

		void SetupGfx();

		Immediate()
		  : accessMgr(1) {}

		static Immediate& Get();
		friend class VulkanCommandBuffer;
		friend class VulkanModule;
	};

	//THIS IS NOT THREAD-SAFE
	class VulkanCommandBuffer : public CommandBuffer {
	  public:
		VulkanCommandBuffer();
		~VulkanCommandBuffer();
		VulkanCommandBuffer(VulkanCommandBuffer&&);
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&&);

		vk::CommandBuffer& vk() {
			return imm.get().cmd;
		}

		void Execute() override;

	  protected:
		std::reference_wrapper<Immediate> imm;
		std::promise<void> promise;

		void StartRendering(glm::vec3 clearColor) override;
		void EndRendering() override;

		friend class VulkanGPU;
		friend class VulkanModule;
	};

	class VulkanGPU final : public GPUManager::Impl {
	  public:
		std::shared_future<void> SubmitCmdBuffer(std::unique_ptr<CommandBuffer>&& cmd) override;
		void RunloopStart() override {}
		void RunloopStop() override {}
		void RunloopIteration() override;

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