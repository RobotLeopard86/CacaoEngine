#pragma once

#include <thread>
#include <map>
#include <vector>
#include <chrono>

#include "Scripts/Script.hpp"
#include "Graphics/Rendering/RenderObjects.hpp"

namespace Cacao {
	/**
	 * @brief Controls dynamic ticks
	 */
	class DynTickController {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static DynTickController* GetInstance();

		/**
		 * @brief Start the controller
		 * @details Spawns a thread to run the controller on
		 *
		 * @note This function is called by the engine during startup
		 *
		 * @throws Exception If the controller was already started
		 */
		void Start();

		/**
		 * @brief Stop the controller
		 * @details Stops the controller and joins its thread
		 *
		 * @note This function is called by the engine during shutdown
		 *
		 * @throws Exception If the controller was not started
		 */
		void Stop();

	  private:
		//Singleton members
		static DynTickController* instance;
		static bool instanceExists;

		//Run the tick controller
		void Run(std::stop_token stopTkn);

		bool isRunning;

		std::jthread* thread;

		std::vector<std::shared_ptr<Component>> tickScriptList;
		double timestep;

		DynTickController()
		  : isRunning(false), thread(nullptr) {}

		void LocateComponents(std::shared_ptr<Entity> e, std::function<void(std::shared_ptr<Component>)> maybeMatch);
	};
}
