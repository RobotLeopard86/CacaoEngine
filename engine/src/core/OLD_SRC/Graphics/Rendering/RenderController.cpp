#include "Graphics/Rendering/RenderController.hpp"

#include "Graphics/Window.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"

namespace Cacao {
	//Required static variable initialization
	RenderController* RenderController::instance = nullptr;
	bool RenderController::instanceExists = false;

	//Singleton accessor
	RenderController* RenderController::Get() {
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
		CheckException(std::this_thread::get_id() == Engine::Get()->GetMainThreadID(), Exception::GetExceptionCodeFromMeaning("BadThread"), "Render controller must be run on the engine thread!");

		auto uiRerenderVar = std::getenv("CACAO_ALWAYS_UI_RERENDER");
		bool alwaysRerenderUI = (uiRerenderVar != nullptr && std::string(uiRerenderVar).compare("YES") == 0);

		//Run while the engine does
		while(Engine::Get()->IsRunning()) {
			//Update window
			Window::Get()->Update();

			//Update graphics state (mostly for immediate-mode backends)
			UpdateGraphicsState();

			//Run main thread tasks
			{
				std::lock_guard lk(Engine::Get()->mainThreadTaskMutex);
				while(!Engine::Get()->mainThreadTasks.empty()) {
					auto task = Engine::Get()->mainThreadTasks.front();
					try {
						task.func();
					} catch(...) {
						task.status->set_exception(std::current_exception());
					}
					task.status->set_value();
					Engine::Get()->mainThreadTasks.pop();
				}
			}

			//Acquire a lock on the queue
			std::unique_lock<std::mutex> lock(fqMutex);

			//Discard frames if we're too far behind
			int maxFrameLag = Engine::Get()->cfg.maxFrameLag;
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
				if(Engine::Get()->GetGlobalUIView()->GetScreen() && (Engine::Get()->GetGlobalUIView()->GetScreen()->IsDirty() || alwaysRerenderUI)) {
					Engine::Get()->GetGlobalUIView()->Render();
				}

				//Render the frame
				ProcessFrame(next);

				//Present rendered frame to window
				Window::Get()->Present();
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
