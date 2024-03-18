#include "Graphics/Rendering/RenderController.hpp"

#include "Graphics/Window.hpp"

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

	void RenderController::Start(){
		if(isRunning) {
			Logging::EngineLog("Cannot start the already started rendering controller!", LogLevel::Error);
			return;
		}
		isRunning = true;
		//Create thread to run controller
		thread = new std::jthread(BIND_MEMBER_FUNC(RenderController::Run));
	}

	void RenderController::Stop(){
		if(!isRunning) {
			Logging::EngineLog("Cannot stop the not started rendering controller!", LogLevel::Error);
			return;
		}
		//Stop run thread
		thread->request_stop();
		cvar.notify_one();
		thread->join();

		//Delete thread object
		delete thread;
		thread = nullptr;

		isRunning = false;
	}

	void RenderController::Run(std::stop_token stopTkn) {
		//Initialize the backend
		Init();

		//Run while we haven't been asked to stop
		while(!stopTkn.stop_requested()){
			//Acquire a lock on the queue
			std::unique_lock<std::mutex> lock(fqMutex);

			//Wait while the frame or render context job queue is empty
			cvar.wait(lock, [this, stopTkn](){
				return (stopTkn.stop_requested() || !this->frameQueue.empty());
			});

			//Process frames
			while(!frameQueue.empty()) {
				//Acquire the next frame
				Frame& next = frameQueue.front();

				//Render the frame
				Render(next);

				//Present rendered frame to window
				Window::GetInstance()->Present();

				//Remove frame from the queue
				frameQueue.pop();
			}

			//Release lock
			lock.unlock();
		}
	}
}
