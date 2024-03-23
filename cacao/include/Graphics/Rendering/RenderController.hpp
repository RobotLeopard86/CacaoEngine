#pragma once

#include "RenderObjects.hpp"

#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace Cacao {
	//Controller for rendering frames
	class RenderController {
	public:
		//Get the instance or create one if it doesn't exist.
		static RenderController* GetInstance();

		//Run the tick controller on the calling thread
		void Run();

		//Enqueue a frame for rendering
		void EnqueueFrame(Frame& frame) {
			//Add a frame to the queue
			{
				std::lock_guard guard(fqMutex);
				frameQueue.push(frame);
			}
			cvar.notify_one();
		}
	private:
		//Singleton members
		static RenderController* instance;
		static bool instanceExists;

		RenderController() {}

		//To be implemented by backend
		void Init();
		void Render(Frame& frame);

		//Queue of frames to render
		std::queue<Frame> frameQueue;

		//Thread safety constructs
		std::mutex fqMutex;
		std::condition_variable cvar;
	};
}
