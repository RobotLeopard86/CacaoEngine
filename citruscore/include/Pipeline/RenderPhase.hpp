#pragma once

#include <queue>

#include "RenderJob.hpp"

namespace Citrus {
	//Manager for the render phase of the frame pipeline
	class RenderPhase {
	public:
		//Get the instance or create one if it doesn't exist.
		static RenderPhase* GetInstance();

		//Run this phase
		//Requires at least one render job to be in queue
		void Run();

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

		RenderPhase() {}
	}
}
