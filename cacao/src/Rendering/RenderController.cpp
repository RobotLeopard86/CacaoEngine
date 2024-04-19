#include "Graphics/Rendering/RenderController.hpp"

#include "Graphics/Window.hpp"
#include "Core/Engine.hpp"

namespace Cacao {
	//Required static variable initialization
	RenderController* RenderController::instance = nullptr;
	bool RenderController::instanceExists = false;

	//Singleton accessor
	RenderController* RenderController::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new RenderController();
			instanceExists = true;
		}

		return instance;
	}

	void RenderController::Run() {
		EngineAssert(isInitialized, "Render controller must be initialized prior to running!");

		//Run while the engine does
		while(Engine::GetInstance()->IsRunning()){
			//Update window and graphics state
			Window::GetInstance()->Update();
			UpdateGraphicsState();
	
			//Acquire a lock on the queue
			std::unique_lock<std::mutex> lock(fqMutex);

			//Process frame if it exists
			if(!frameQueue.empty()) {
				//Acquire the next frame
				Frame& next = frameQueue.front();

				//Release lock
				lock.unlock();

				//Render the frame
				ProcessFrame(next);

				//Present rendered frame to window
				Window::GetInstance()->Present();
				Window::GetInstance()->Update();

				//Remove frame from the queue
				lock.lock();
				frameQueue.pop();
				lock.unlock();
			} else {
				//Release lock and wait for a bit to avoid wasting CPU cycles
				lock.unlock();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}
}
