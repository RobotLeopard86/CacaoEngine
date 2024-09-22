#pragma once

#include <atomic>
#include <string>
#include <map>

#include "dynalo/dynalo.hpp"
#include "thread_pool/thread_pool.h"

#include "EngineConfig.hpp"
#include "Utilities/MiscUtils.hpp"
#include "UI/UIView.hpp"

/**
 * @brief Shorthand engine thread pool type.
 * @see dp::thread_pool for its API.
 */
using thread_pool = dp::thread_pool<dp::details::default_function_type, std::jthread>;

namespace Cacao {
	///@brief Singleton representing the engine
	class Engine {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static Engine* GetInstance();

		/**
		 * @brief Start the engine
		 * @details This function is called automatically to start everything, and blocks until engine shutdown
		 */
		void Run();

		/**
		 * @brief Stop the engine
		 * @details This function will shut down all engine systems and exit cleanly
		 */
		void Stop();

		///@brief Access the thread pool
		std::shared_ptr<thread_pool> GetThreadPool() {
			return threadPool;
		}

		///@brief Access the global UI view
		std::shared_ptr<UIView> GetGlobalUIView() {
			return uiView;
		}

		/**
		 * @brief Check if the engine is running
		 *
		 * @return If the engine is running
		 */
		bool IsRunning() {
			return run;
		}

		/**
		 * @brief Check if the engine is shutting down
		 *
		 * @return If the engine is shutting down
		 */
		bool IsShuttingDown() {
			return shuttingDown;
		}

		/**
		 * @brief Get the thread ID that @ref Run was called on
		 * @details Useful for identifying the main thread
		 */
		std::thread::id GetThreadID() {
			return threadID;
		}

		///@brief @a Engine configuration values
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
		std::unique_ptr<dynalo::library> gameLib;

		//Top-level UI view
		std::shared_ptr<UIView> uiView;

		Engine() {}

		//Run the core startup and shutdown systems of the engine (render thread has to be on main and running early to process graphics stuff, so startup work gets forked off)
		void CoreStartup();
		void CoreShutdown();

		//To be implemented by backend
		//Register backend-specific exception codes
		void RegisterBackendExceptions();
	};
}