#include "Cacao/FrameProcessor.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Log.hpp"
#include "Cacao/TickController.hpp"
#include "Cacao/Window.hpp"
#include "SingletonGet.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <thread>

#include "glm/exponential.hpp"
#include "glm/glm.hpp"

namespace Cacao {
	struct FrameProcessor::Impl {
		void Runloop(std::stop_token stop);

		std::unique_ptr<std::jthread> thread;
	};

	FrameProcessor::FrameProcessor()
	  : running(false) {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
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

		running = true;
	}

	void FrameProcessor::Stop() {
		Check<BadInitStateException>(running, "The frame processor must be running when Stop is called!");

		running = false;

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
			while(Window::Get().IsMinimized()) {
				std::this_thread::yield();
				if(stop.stop_requested()) return;
			}

			Logger::Engine(Logger::Level::Info) << "we wanna snapshot";

			//Request a snapshot of the world state
			TickController::Get().snapshotControl.request.store(true, std::memory_order_release);

			//Block until the tick controller grants the request
			while(!TickController::Get().snapshotControl.grant.try_acquire()) {
				std::this_thread::yield();
				if(stop.stop_requested()) return;
			}

			Logger::Engine(Logger::Level::Info) << "yeah we can have the snapshot (woohoo)";

			//Now we are safe to read the world state
			{
				//Lock the world state
				std::unique_lock lock(TickController::Get().snapshotControl.mutex);

				//TODO: World read logic
			}

			//Allow tick controller to resume
			//It has been blocking on this semaphore
			TickController::Get().snapshotControl.done.release();

			Logger::Engine(Logger::Level::Info) << "now we done with the snapshot";

			//Clear color
			constexpr glm::vec3 clearColor {0x00, 0xAC, 0xE6};
			const static glm::vec3 clearColorLinear {srgbChannel2Linear(clearColor.r / 255), srgbChannel2Linear(clearColor.g / 255), srgbChannel2Linear(clearColor.b / 255)};

			Logger::Engine(Logger::Level::Info) << "time to give birth (hi buffy)";

			//Setup command buffer
			std::unique_ptr<CommandBuffer> cmd = CommandBuffer::Create();
			Logger::Engine(Logger::Level::Info) << "start the render";
			cmd->StartRendering(clearColorLinear);
			Logger::Engine(Logger::Level::Info) << "end the render";
			cmd->EndRendering();

			Logger::Engine(Logger::Level::Info) << "i send thee to execution!";

			//Execute command buffer
			std::shared_future<void> submission = GPUManager::Get().Submit(std::move(cmd));
			while(submission.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout) {
				Logger::Engine(Logger::Level::Info) << "thats why we waiting (waiting) waiting on da world to change";
				bool wantStop = stop.stop_requested();
				Logger::Engine(Logger::Level::Info) << "and is it time to stop yet? " << (wantStop ? "oh yeah" : "no way");
				if(wantStop) return;
			}

			Logger::Engine(Logger::Level::Info) << "buffy the buffer is done";
		}

		Logger::Engine(Logger::Level::Info) << "ooga booga framey is done";
	}
}