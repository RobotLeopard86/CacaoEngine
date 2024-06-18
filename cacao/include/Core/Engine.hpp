#pragma once

#include <atomic>
#include <string>
#include <map>

#include "dynalo/dynalo.hpp"
#include "thread_pool/thread_pool.h"

#include "EngineConfig.hpp"
#include "Utilities/MiscUtils.hpp"

using thread_pool = dp::thread_pool<dp::details::default_function_type, std::jthread>;

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
		std::shared_ptr<thread_pool> GetThreadPool() {
			return threadPool;
		}

		//Is the engine running?
		bool IsRunning() {
			return run;
		}

		//Is the engine shutting down?
		bool IsShuttingDown() {
			return shuttingDown;
		}

		//Get the thread ID of the engine
		std::thread::id GetThreadID() {
			return threadID;
		}

		//Engine config properties
		EngineConfig cfg;

	  private:
		//Singleton members
		static Engine* instance;
		static bool instanceExists;

		//Should the engine run?
		std::atomic_bool run;

		//Is shutdown running?
		std::atomic_bool shuttingDown;

		//Thread pool
		std::shared_ptr<thread_pool> threadPool;

		//Engine thread ID
		std::thread::id threadID;

		//Game library
		dynalo::library* gameLib;

		Engine() {}

		//Run the core startup and shutdown systems of the engine on separate thread (main thread handles rendering)
		void CoreStartup();
		void CoreShutdown();

		//To be implemented by backend
		//Register backend-specific exception codes
		void RegisterBackendExceptions();
	};
}