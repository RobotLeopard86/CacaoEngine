#pragma once

#include "RenderObjects.hpp"

#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>

namespace Cacao {
	//Controller for rendering frames
	class RenderController {
	public:
		//Get the instance or create one if it doesn't exist.
		static RenderController* GetInstance();

		//Start the controller
		void Start();

		//Stop the controller
		void Stop();

		//Enqueue a frame for rendering
		void EnqueueFrame(Frame& frame) {
			//Add a task to the queue
			{
				std::lock_guard guard(fqMutex);
				frameQueue.push(frame);
			}
			fqCvar.notify_one();
		}
	private:
		//Singleton members
		static RenderController* instance;
		static bool instanceExists;

		RenderController() 
			: isRunning(false), thread(nullptr) {}

		//Run the tick controller
		void Run(std::stop_token stopTkn);

		//To be implemented by backend
		void Render(Frame frame);
		void Init();

		bool isRunning;
		std::jthread* thread;

		//Queue of frames to render
		std::queue<Frame> frameQueue;

		//Thread safety constructs
		std::mutex fqMutex;
		std::condition_variable fqCvar;
	};
}
