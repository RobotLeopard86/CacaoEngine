#pragma once

#include "RenderObjects.hpp"
#include "Core/Engine.hpp"

#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace Cacao {
	/**
	 * @brief Frame renderer
	 *
	 * @note For use by the engine only
	 */
	class RenderController {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static RenderController* Get();

		/**
		 * @brief Run the controller on the calling thread
		 *
		 * @note Called automatically after engine startup
		 */
		void Run();

		/**
		 * @brief Enqueue a frame for rendering
		 *
		 * @param frame The frame to enqueue
		 */
		void EnqueueFrame(std::shared_ptr<Frame>& frame) {
			if(!Engine::Get()->IsShuttingDown()) {
				//Add a frame to the queue
				std::lock_guard guard(fqMutex);
				frameQueue.push(frame);
			}
		}

		/**
		 * @brief Clear the frame queue
		 */
		void ClearRenderQueue() {
			std::unique_lock<std::mutex> lock(fqMutex);
			while(!frameQueue.empty()) frameQueue.pop();
			lock.unlock();
		}

		/**
		 * @brief Initialize the rendering backend
		 *
		 * @throws Exception If the backend is already initialized
		 */
		void Init();

		/**
		 * @brief Shut down the rendering backend
		 *
		 * @throws Exception If the backend is not initialized
		 */
		void Shutdown();

	  private:
		//Singleton members
		static RenderController* instance;
		static bool instanceExists;

		RenderController()
		  : isInitialized(false) {}

		//Process a frame for drawing
		void ProcessFrame(std::shared_ptr<Frame> frame);

		//Update the graphics state
		void UpdateGraphicsState();

		//Wait for GPU idle before termination
		void WaitGPUIdleBeforeTerminate();

		//Queue of frames to render
		std::queue<std::shared_ptr<Frame>> frameQueue;

		//Thread safety constructs
		std::mutex fqMutex;

		bool isInitialized;
	};
}
