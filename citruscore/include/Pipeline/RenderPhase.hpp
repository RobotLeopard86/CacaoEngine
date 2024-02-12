#pragma once

#include <queue>
#include <future>

#include "glm/mat4x4.hpp"

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

		//Shuts down the render phase
		//Calling GetInstance again will re-create the instance
		void Shutdown() {
			BeforeShutdown();
			free(nativeData);
			instanceExists = false;
			delete this;
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
		void Prerender(glm::mat4 pm, glm::mat4 vm);
		void BeforeShutdown();

		void* nativeData;

		RenderPhase();
	};
}
