#pragma once

#include <atomic>

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
void OnDynamicTick(float timestep);