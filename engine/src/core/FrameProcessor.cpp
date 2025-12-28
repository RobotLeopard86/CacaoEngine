#include "Cacao/FrameProcessor.hpp"
#include "Cacao/Event.hpp"
#include "Cacao/EventConsumer.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/TickController.hpp"
#include "Cacao/Window.hpp"
#include "SingletonGet.hpp"
#include "ImplAccessor.hpp"
#include "impl/PAL.hpp"
#include "impl/FrameProcessor.hpp"

#include <atomic>
#include <thread>

#include "glm/exponential.hpp"
#include "glm/glm.hpp"

namespace Cacao {
	FrameProcessor::FrameProcessor()
	  : running(false) {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
		impl->numFramesInFlight.store(0);
		impl->swapchainRegen.store(false);
	}

	FrameProcessor::~FrameProcessor() {
		if(running) Stop();
	}

	CACAOST_GET(FrameProcessor)

	void FrameProcessor::Start() {
		Check<BadInitStateException>(!running, "The frame processor must not be running when Start is called!");
		Check<BadStateException>(GPUManager::Get().IsRunning(), "The GPU manager must be running when Start is called on the frame processor!");

		//Start runloop on background thread
		auto runloop = [this](std::stop_token stop) { impl->Runloop(stop); };
		impl->thread = std::make_unique<std::jthread>(runloop);

		//Subscribe swapchain recreation event consumer
		impl->resizeConsumer = EventConsumer([this](Event&) {
			impl->swapchainRegen.store(true);
		});
		EventManager::Get().SubscribeConsumer("WindowResize", impl->resizeConsumer);
		EventManager::Get().SubscribeConsumer("INTERNAL-RegenSwapchain", impl->resizeConsumer);

		running = true;
	}

	void FrameProcessor::Stop() {
		Check<BadInitStateException>(running, "The frame processor must be running when Stop is called!");

		running = false;

		//Unsubscribe event consumer
		EventManager::Get().UnsubscribeConsumer("WindowResize", impl->resizeConsumer);
		EventManager::Get().UnsubscribeConsumer("INTERNAL-RegenSwapchain", impl->resizeConsumer);

		//Signal run loop stop
		impl->thread->request_stop();
		impl->thread->join();
	}

	float srgbChannel2Linear(float c) {
		//This is the sRGB -> linear conversion formula
		if(c <= 0.04045f)
			return c / 12.92f;
		else {
			const float a = (c + 0.055f) / 1.055f;
			return static_cast<float>(std::pow(a, 2.4));
		}
	}

	void FrameProcessor::Impl::Runloop(std::stop_token stop) {
		while(!stop.stop_requested()) {
			//If the window is minimized, we can't render, so no point in working
			//Same for if there are too many frames in flight
			while(Window::Get().IsMinimized() || numFramesInFlight > IMPL(GPUManager).MaxFramesInFlight()) {
				if(stop.stop_requested()) return;
			}

			//Request a snapshot of the world state
			TickController::Get().snapshotControl.request.store(true, std::memory_order_release);

			//Block until the tick controller grants the request
			while(!TickController::Get().snapshotControl.grant.try_acquire()) {
				if(stop.stop_requested()) return;
			}

			//Now we are safe to read the world state
			//TODO: World read logic

			//Allow tick controller to resume
			//It has been blocking on this semaphore
			TickController::Get().snapshotControl.done.release();

			//If needed, regenerate swapchain
			if(swapchainRegen.load(std::memory_order_relaxed)) {
				if(numFramesInFlight == 0) {
					swapchainRegen.store(false);
					IMPL(GPUManager).GenSwapchain();
				} else {
					continue;
				}
			}

			//Setup command buffer
			//We use the internal API so we can do rendering setup
			std::unique_ptr<CommandBuffer> cmd = IMPL(PAL).mod->CreateCmdBuffer();
			if(!cmd->SetupContext(true)) continue;

			//Clear color
			constexpr glm::vec3 clearColor {0x00, 0xAC, 0xE6};
			const static glm::vec3 clearColorLinear {srgbChannel2Linear(clearColor.r / 255), srgbChannel2Linear(clearColor.g / 255), srgbChannel2Linear(clearColor.b / 255)};

			//Record commands
			cmd->StartRendering(clearColorLinear);
			cmd->EndRendering();

			//Execute command buffer
			try {
				++numFramesInFlight;
				std::shared_future<void> fut = GPUManager::Get().Submit(std::move(cmd));
				if(IMPL(GPUManager).UsesImmediateExecution()) {
					fut.get();
					if(numFramesInFlight > 0) --numFramesInFlight;
				}
			} catch(const MiscException&) {}
		}
	}
}