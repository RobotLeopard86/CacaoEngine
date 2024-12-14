#include "Graphics/Rendering/RenderController.hpp"

#include "Graphics/Window.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"

namespace Cacao {
	//Required static variable initialization
	RenderController* RenderController::instance = nullptr;
	bool RenderController::instanceExists = false;

	//Singleton accessor
	RenderController* RenderController::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == nullptr) {
			//Create instance
			instance = new RenderController();
			instanceExists = true;
		}

		return instance;
	}

	void RenderController::Run() {
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Uninitialized render controller cannot be run!");
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetMainThreadID(), Exception::GetExceptionCodeFromMeaning("BadThread"), "Render controller must be run on the engine thread!");

		//Run while the engine does
		while(Engine::GetInstance()->IsRunning()) {
			//Update window
			Window::GetInstance()->Update();

			//Update graphics state (mostly for immediate-mode backends)
			UpdateGraphicsState();

			//Run main thread tasks
			{
				std::lock_guard lk(Engine::GetInstance()->mainThreadTaskMutex);
				while(!Engine::GetInstance()->mainThreadTasks.empty()) {
					auto task = Engine::GetInstance()->mainThreadTasks.front();
					try {
						task.func();
					} catch(...) {
						task.status->set_exception(std::current_exception());
					}
					task.status->set_value();
					Engine::GetInstance()->mainThreadTasks.pop();
				}
			}

			//Acquire a lock on the queue
			std::unique_lock<std::mutex> lock(fqMutex);

			//Discard frames if we're too far behind
			int maxFrameLag = Engine::GetInstance()->cfg.maxFrameLag;
			if(frameQueue.size() > maxFrameLag) {
				while(frameQueue.size() > 1) {
					frameQueue.pop();
				}
			}

			//Process frame if it exists
			if(!frameQueue.empty()) {
				//Acquire the next frame and pop it
				std::shared_ptr<Frame> next = frameQueue.front();
				frameQueue.pop();

				//Release lock
				lock.unlock();

				//If the global UI is dirty, re-render
				if(Engine::GetInstance()->GetGlobalUIView()->GetScreen() /*&& Engine::GetInstance()->GetGlobalUIView()->GetScreen()->IsDirty()*/) {
					Engine::GetInstance()->GetGlobalUIView()->Render();
				}

				//Render the frame
				ProcessFrame(next);

				//Present rendered frame to window
				Window::GetInstance()->Present();
			} else {
				//Release lock and wait for a bit to avoid wasting CPU cycles
				lock.unlock();
				std::this_thread::sleep_for(std::chrono::microseconds(1));
			}
		}

		//Make sure the GPU is idle before termination
		WaitGPUIdleBeforeTerminate();
	}
}
