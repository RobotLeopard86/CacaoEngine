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

		//Run while the window is open
		while(Engine::GetInstance()->IsRunning()){
			//Acquire a lock on the queue
			std::unique_lock<std::mutex> lock(fqMutex);

			//Wait while the frame or render context job queue is empty
			cvar.wait(lock, [this](){
				if(Window::GetInstance()->IsOpen()) Window::GetInstance()->Update();
				return (Engine::GetInstance()->IsRunning() || !this->frameQueue.empty());
			});

			//Process frames
			while(!frameQueue.empty() && Engine::GetInstance()->IsRunning()) {
				//Acquire the next frame
				Frame& next = frameQueue.front();

				//Render the frame
				ProcessFrame(next);

				//Present rendered frame to window
				Window::GetInstance()->Present();
				Window::GetInstance()->Update();

				//Remove frame from the queue
				frameQueue.pop();
			}

			//Release lock
			lock.unlock();
		}
	}
}
