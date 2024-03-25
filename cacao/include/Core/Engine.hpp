#pragma once

#include <atomic>
#include <string>
#include <map>

#define BS_THREAD_POOL_ENABLE_PAUSE
#include "BS_thread_pool.hpp"

#include "EngineConfig.hpp"
#include "Utilities/MiscUtils.hpp"

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

		//Request any native data needed for a graphics context
		//Should only be called by thread pools, hence the poolID name
		//Do not use this function otherwise (it will yell at you in the console)
		NativeData* RequestGraphicsContext(size_t poolID);
		
		//Set up a graphics context for use
		//Backed implementation required
		void SetupGraphicsContext(NativeData* context);

		//Is the engine running?
		bool IsRunning() { return run; }

		//Retrieve thread ID of the engine thread
		std::thread::id GetThreadID() { return threadID; }

		//Engine config properties
		EngineConfig cfg;
	private:
		//Singleton members
		static Engine* instance;
		static bool instanceExists;

		//Should the engine run?
		std::atomic_bool run;

		//Thread pool
		BS::thread_pool threadPool;

		//Thread ID
		std::thread::id threadID;

		Engine() {}

		//List of loaned graphics contexts
		std::map<size_t, NativeData*> loanedContexts;

		//Backend implementation required
		NativeData* _CreateGraphicsContext();
		void _DeleteGraphicsContext(NativeData* context);
	};
}