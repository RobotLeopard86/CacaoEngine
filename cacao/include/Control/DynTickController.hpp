#pragma once

#include <thread>
#include <chrono>

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

		DynTickController() 
			: isRunning(false), thread(nullptr) {}
	};
}
