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
		CheckException(isInitialized, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Uninitialized render controller cannot be run!")

		//Run while the engine does
		while(Engine::GetInstance()->IsRunning()) {
			//Update window and graphics state
			Window::GetInstance()->Update();
			UpdateGraphicsState();

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

				std::chrono::steady_clock::time_point fb = std::chrono::steady_clock::now();

				//Release lock
				lock.unlock();

				//If the global UI is dirty, re-render
				if(Engine::GetInstance()->GetGlobalUIView()->GetScreen() && Engine::GetInstance()->GetGlobalUIView()->GetScreen()->IsDirty()) {
					Engine::GetInstance()->GetGlobalUIView()->Render();
				}

				//Render the frame
				ProcessFrame(next);

				//Present rendered frame to window
				Window::GetInstance()->Present();

				std::stringstream loggo;
				loggo << "Render took " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - fb);
				Logging::EngineLog(loggo.str(), LogLevel::Trace);
			} else {
				//Release lock and wait for a bit to avoid wasting CPU cycles
				lock.unlock();
				std::this_thread::sleep_for(std::chrono::microseconds(1));
			}
		}
	}
}
