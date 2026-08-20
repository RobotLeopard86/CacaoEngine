#pragma once

#include "Cacao/GPU.hpp"

#include <thread>
#include <atomic>

#define FPS_AVG_WINDOW 5

namespace Cacao {
	using clock = std::chrono::steady_clock;

	class GPUManager::Impl {
	  public:
		virtual std::shared_future<void> SubmitCmdBuffer(std::unique_ptr<CommandBuffer>&& cmd) = 0;
		virtual void RunloopStart() = 0;
		virtual void RunloopStop() = 0;
		virtual void RunloopIteration() = 0;

		virtual ~Impl() = default;

		void Runloop(std::stop_token stop);
		void FrameGen();
		std::unique_ptr<std::jthread> thread;

		virtual bool UsesImmediateExecution() = 0;
		virtual unsigned int MaxFramesInFlight() {
			return UINT32_MAX;
		}
		virtual void GenSwapchain() = 0;
		virtual void WaitIdle() {}

		//VSync state management
		struct VSyncRequest {
			std::atomic_bool needChange;
			bool value;
		} vsreq;

		//Current frames-in-flight tracker
		std::atomic<unsigned int> numFramesInFlight;

		//Frame generation toggle
		//This is false most of the time but set to true during shutdown to avoid asset unloading problems
		std::atomic_bool masterFrameGenDisable;

		//Swapchain regeneration state
		std::atomic_bool swapchainRegen;
		EventConsumer resizeConsumer;

		//FPS measurement
		unsigned int counter;
		clock::time_point lastSecond;
		std::array<unsigned int, FPS_AVG_WINDOW> fpsMeasures;

		//Custom rendering callback state
		std::unordered_map<xg::Guid, std::function<void(std::unique_ptr<CommandBuffer>&)>> callbacks;
		std::unordered_map<GPUManager::Phase, std::pair<std::vector<xg::Guid>, std::vector<xg::Guid>>> mappings;
		std::unordered_map<xg::Guid, std::pair<GPUManager::Phase, bool>> reverseMappings;

		//Skybox info
		std::shared_ptr<Mesh> skyCube;
		std::shared_ptr<Shader> skyShader;
		std::shared_ptr<Material> skyMat;
		std::string lastKnownSkybox;

		//Current blocking future (for immediate-execution)
		std::optional<std::shared_future<void>> immediateExecutionTask;
	};
}