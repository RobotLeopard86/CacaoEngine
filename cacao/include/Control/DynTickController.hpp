#pragma once

#include <thread>
#include <map>
#include <vector>
#include <chrono>

#include "Scripts/Script.hpp"
#include "Graphics/Rendering/RenderObjects.hpp"

namespace Cacao {
	//Controller for running dynamic ticks
	class DynTickController {
	public:
		//Get the instance or create one if it doesn't exist.
		static DynTickController* GetInstance();

		//Start the controller
		void Start();

		//Stop the controller
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
		std::vector<RenderObject> tickRenderList;
		double timestep;

		DynTickController() 
			: isRunning(false), thread(nullptr) {}
	};
}
