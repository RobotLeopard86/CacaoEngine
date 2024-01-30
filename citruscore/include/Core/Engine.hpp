#pragma once

#include <atomic>
#include <chrono>
#include <string>

namespace Citrus {
	//Singleton representing the engine
	class Engine {
	public:
		//Get an instance of the engine, or create one if it doesn't exist
		static Engine* GetInstance();

		//Run the engine
		void Run();

		//Stop the engine
		void Stop();
	private:
		//Singleton members
		static Engine* instance;
		static bool instanceExists;

		//Should the engine run?
		std::atomic_bool run;

		Engine();

		//Engine start time (used for calculating elapsed time)
		std::chrono::time_point<std::chrono::steady_clock> lastFrame;
	};
}

//Client code hooks
//These are set to nothing by default
//This is so they are optional to implement

//Runs once on engine startup
void OnStartup();

//Runs once on engine shutdown
void OnShutdown();

//Runs every fixed tick
void OnFixedTick();

//Runs every dynamic tick
void OnDynamicTick(double timestep);

//Get the target window title
std::string GetWindowTitle();