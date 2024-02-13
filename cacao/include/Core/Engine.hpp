#pragma once

#include <atomic>
#include <chrono>
#include <string>

#include "BS_thread_pool.hpp"

namespace Cacao {
	//Singleton representing the engine
	class Engine {
	public:
		//Get an instance of the engine, or create one if it doesn't exist
		static Engine* GetInstance();

		//Run the engine
		void Run();

		//Stop the engine
		void Stop();

		//Access the thread pool
		BS::thread_pool& GetThreadPool() { return threadPool; }
	private:
		//Singleton members
		static Engine* instance;
		static bool instanceExists;

		//Should the engine run?
		std::atomic_bool run;

		//Thread pool
		BS::thread_pool threadPool;

		Engine() {}

		//Engine start time (used for calculating elapsed time)
		std::chrono::time_point<std::chrono::steady_clock> lastFrame;
	};
}