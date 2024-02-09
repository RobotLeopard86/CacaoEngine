#pragma once

#include <queue>
#include <future>

#include "RenderJob.hpp"

namespace Citrus {
	//Manager for the render phase of the frame pipeline
	class RenderPhase {
	public:
		//Get the instance or create one if it doesn't exist.
		static RenderPhase* GetInstance();

		//Run this phase asynchronously
		//Requires at least one render job to be in queue
		std::future<void> Run() {
			return std::async(std::launch::async, [this](){
				this->Run();
			});
		}

		//Enqueue a job for rendering
		void QueueJob(RenderJob job) {
			renderQueue.push(job);
		}
	private:
		//Singleton members
		static RenderPhase* instance;
		static bool instanceExists;

		//Render job queue
		std::queue<RenderJob> renderQueue;

		void _Run();

		//Needs backend implementation
		//Execute a render command
		void ExecuteRenderCmd(RenderCmd cmd);

		RenderPhase() {}
	};
}
