#pragma once

#include "Cacao/GPU.hpp"
#include "Cacao/Material.hpp"
#include "impl/PAL.hpp"
#include "BackendCommon.hpp"

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

namespace Cacao {
	template<typename T>
	struct Allocated {
	  public:
		vma::Allocation alloc;
		T obj;

		Allocated() {}
		Allocated(std::pair<vma::Allocation, T> p) : alloc(p.first), obj(p.second) {}
	};

	struct ViewImage : public Allocated<vk::Image> {
		vk::ImageView view;

		ViewImage() {}
		ViewImage(std::pair<vma::Allocation, vk::Image> p) : Allocated<vk::Image>(p) {}
	};

	struct MappedBuffer : Allocated<vk::Buffer> {
		void* mem;

		MappedBuffer() {}
		MappedBuffer(std::pair<vma::Allocation, vk::Buffer> p) : Allocated<vk::Buffer>(p) {}
	};

	struct Sync {
		vk::Semaphore semaphore;
		uint64_t doneValue = 0;
	};

	class TransientCommandContext {
	  public:
		vk::CommandPool pool;
		Sync sync;

		static TransientCommandContext* Get();
		static void Cleanup();

		TransientCommandContext(const TransientCommandContext&) = delete;
		TransientCommandContext& operator=(const TransientCommandContext&) = delete;
		TransientCommandContext(TransientCommandContext&& o)
		  : pool(std::exchange(o.pool, {})), sync(std::exchange(o.sync, {})) {}
		TransientCommandContext& operator=(TransientCommandContext&& o) {
			pool = std::exchange(o.pool, {});
			sync = std::exchange(o.sync, {});
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
		//Rendering control
		vk::Fence inFlight;
		vk::Semaphore rendered;
		uint32_t imageIndex;
		Sync sync;

		//Engine descriptors
		MappedBuffer globals;
		vk::DescriptorSet set;
	};

	//THIS IS NOT THREAD-SAFE
	class VulkanCommandBuffer : public CommandBuffer {
	  public:
		VulkanCommandBuffer() {}
		~VulkanCommandBuffer();
		VulkanCommandBuffer(VulkanCommandBuffer&&);
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&&);

		void Execute() override;
		Sync& GetSync();

		vk::CommandBuffer cmd;

		void DrawMesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, Transform transform) override;
		void BindSet(const vk::PipelineLayout& layout);

	  protected:
		//Contexts
		TransientCommandContext* transient = nullptr;
		RenderCommandContext* render = nullptr;

		//Command buffer pool
		vk::CommandPool* poolPtr = nullptr;

		//Promise
		std::promise<void> promise;

		//Commands
		bool SetupContext(bool rendering) override;
		void StartRendering(glm::vec3 clearColor) override;
		void EndRendering() override;
		void UpdateEngineData(std::shared_ptr<Camera> cam, bool worldRefresh) override;

		friend class VulkanGPU;
		friend class VulkanModule;
	};

	class VulkanGPU final : public GPUManager::Impl {
	  public:
		std::shared_future<void> SubmitCmdBuffer(std::unique_ptr<CommandBuffer>&& cmd) override;
		void RunloopStart() override {}
		void RunloopStop() override;
		void RunloopIteration() override;

		bool UsesImmediateExecution() override {
			return false;
		}
		unsigned int MaxFramesInFlight() override;
		void GenSwapchain() override;
		void WaitIdle() override;

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
		Shader::Impl* ConfigureShader() override;
		Material::Impl* ConfigureMaterial() override;
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
		vk::SurfaceCapabilitiesKHR capabilities;
		struct SwapchainData {
			//Swapchain and data
			vk::SwapchainKHR chain;
			vk::Extent2D extent;
			uint16_t cycle = 0;

			//Images
			std::vector<vk::Image> images;
			std::vector<vk::ImageView> views;
			std::vector<ViewImage> depthImages;

			//Contexts
			std::vector<RenderCommandContext> contexts;

			//Acquire semaphore data
			std::vector<vk::Semaphore> acquireSems;
			std::vector<uint16_t> imageSemIndices;
		} swapchain;
		vk::CommandPool renderingPool;
		std::mutex renderPoolMtx;

		//==================== ENGINE DATA DESCRIPTORS ====================
		vk::DescriptorSetLayout engineSetLayout;
		vk::DescriptorPool descriptorPool;

		//==================== MISCELLANEOUS FIELDS ====================
		bool vsync;
		std::mutex queueMtx;
		vk::Format selectedDF;

		VulkanModule()
		  : PALModule("vulkan") {}
		~VulkanModule() {}
	};

	inline std::shared_ptr<VulkanModule> vulkan;

#ifdef HAS_WAYLAND
	void CommitAfterRegen();
#endif

	constexpr glm::mat4 projectionCorrection(
		{1.0f, 0.0f, 0.0f, 0.0f}, //No X change
		{0.0f, -1.0f, 0.0f, 0.0f},//Invert Y
		{0.0f, 0.0f, 0.5f, 0.0f}, //Halve depth range: [-1, 1] (OpenGL range) -> [-0.5, 0.5]
		{0.0f, 0.0f, 0.5f, 1.0f});//Shift depth range: [-0.5, 0.5] => [0, 1] (Vulkan range)
}