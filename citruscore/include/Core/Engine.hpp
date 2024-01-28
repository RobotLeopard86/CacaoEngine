#pragma once

#include "boost/asio.hpp"

namespace Citrus {
	//Singleton representing the engine
	class Engine {
	public:
		//Get an instance of the engine, or create one if it doesn't exist
		static Engine* GetInstance();

		//Run the engine
		void Run();
	private:
		//Singleton members
		static Engine* instance;
		static bool instanceExists;

		//Should the engine run?
		std::atomic_bool run;

		//Fixed tick rate (milliseconds)
		constexpr int fixedTickRate = 50;

		//Fixed tick handler
		void FixedTickHandler(boost::asio::io_context& io);
	};
}

//Client code hooks
//These are set to nothing by default
//This is so they are optional to implement

//Runs once on engine startup
void OnStartup() {}

//Runs once on engine shutdown
void OnShutdown() {}

//Runs every fixed tick
void OnFixedTick() {}

//Runs every dynamic tick
void OnDynamicTick() {}