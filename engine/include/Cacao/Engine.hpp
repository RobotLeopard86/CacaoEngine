#pragma once

#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <filesystem>
#include <thread>
#include <queue>
#include <functional>

#include "Exceptions.hpp"
#include "DllHelper.hpp"
#include "Identity.hpp"

#include "exathread.hpp"

using namespace std::chrono_literals;

namespace Cacao {
	/**
	 * @brief Core engine systems singleton
	 */
	class CACAO_API Engine {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static Engine& Get();

		///@cond
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;
		///@endcond

		//======================= CONFIGURATION =======================

		/**
		 * @brief Configuration values for the engine that may change at runtime
		 */
		struct CACAO_API RuntimeConfig {
			/**
			 * @brief Whether or not to always re-render the UI every frame. Useful for inspecting UI graphics calls in RenderDoc or similar
			 */
			bool alwaysRerenderUI = false;

			/**
			 * @brief The maximum allowed number of ticks per second; use this to throttle updates or set to UINT32_MAX for unlimited
			 */
			unsigned int maxTPS = UINT32_MAX;

			/**
			 * @brief Whether or not to verbosely log all received input events
			 */
			bool verboseInputLogging = false;
		};

		/**
		 * @brief Configuration values that are set for startup and cannot be modified afterwards
		 */
		struct CACAO_API InitConfig {
			/**
			 * @brief The requested backend to try to initialize first, overriding the default setting.
			 */
			std::string initialRequestedBackend;

			/**
			 * @brief The preferred windowing provider to use
			 *
			 * The available options are:
			 * \li \c win32 Windows API (Windows only)
			 * \li \c cocoa Cocoa (MacOS only)
			 * \li \c x11 X11 via XCB (Linux only)
			 * \li \c wayland Wayland (Linux only)
			 *
			 * @note If the provider requested is not available or this string is empty, default behavior will be used
			 */
			std::string preferredWindowProvider;

			/**
			 * @brief Whether or not to suppress console logging output
			 */
			bool suppressConsoleLogging = false;

			/**
			 * @brief Whether or not to suppress file logging output
			 */
			bool suppressFileLogging = false;

			/**
			 * @brief ID of the client application. This should be in reverse-domain format with a PascalCase final segment (e.g. com.example.MyGame), but this is not enforced
			 */
			ClientIdentity clientID;

			/**
			 * @brief Whether to start the frame processor during graphics system initialization or to start it with the engine gameloop
			 *
			 * This is set to @c false by default, which is best for games. However, other users may want to continue rendering
			 * without an active gameloop, thus this option exists.
			 */
			bool startFrameProcessorWithGfxSystem = false;
		};

		/**
		 * @brief Get an immutable reference to the initialization config values
		 *
		 * @return The initialization config values
		 */
		const InitConfig& GetInitConfig() const {
			return icfg;
		}

		/**
		 * @brief Get a mutable reference to the runtime config values
		 *
		 * @return The runtime config values
		 */
		RuntimeConfig& GetRuntimeConfig() {
			return rcfg;
		}

		//===================== UTILITY FUNCTIONS =====================

		/**
		 * @brief Get a path to the data directory where logs and game data should be stored
		 *
		 * This path will be created if it does not exist when this function is called
		 *
		 * @return Data directory path
		 */
		const std::filesystem::path GetDataDirectory();

		/**
		 * @brief Get access to the engine thread pool
		 *
		 * @see https://robotleopard86.github.io/Exathread/stable for the documentation on how to use the pool
		 *
		 * @return A handle to the pool
		 *
		 * @throws BadStateException If the thread pool is not running
		 */
		std::shared_ptr<exathread::Pool> GetThreadPool();

		/**
		 * @brief Run a task on the main thread
		 *
		 * @param func The task to execute
		 * @param args The arguments to the task function
		 *
		 * @return A future that will be fulfilled when the task completes
		 *
		 * @throws BadStateException If the engine is not in the Running state
		 */
		template<typename F, typename... Args, typename R = std::invoke_result_t<F&&, Args&&...>>
			requires std::invocable<F&&, Args&&...>
		std::shared_future<R> RunTaskOnMainThread(F func, Args... args) {
			Check<BadStateException>(state == State::Running, "Engine must be in running state to submit main thread task!");

			//Create a task and get a result future
			std::shared_ptr<std::packaged_task<R()>> task;
			if constexpr(sizeof...(args) == 0) {
				task = std::make_shared<std::packaged_task<R()>>(std::move(func));
			} else {
				//Wrap the function so it doesn't need any arguments
				auto wrapper = std::bind(std::forward<F>(func), std::forward<Args...>(args...));
				task = std::make_shared<std::packaged_task<R()>>(std::move(wrapper));
			}
			std::shared_future<R> result = task->get_future().share();

			//Is this the main thread?
			if(std::this_thread::get_id() == mainThread) {
				(*task)();
				return result;
			}

			//Add task to queue
			{
				std::lock_guard lk(mttQueueMtx);
				mainThreadTasksQueue.emplace([task]() { (*task)(); });
			}

			//Return future
			return result;
		}

		//========================= LIFECYCLE =========================

		/**
		 * @brief The current state of the engine
		 */
		enum class State {
			Dead,
			Alive,
			Ready,
			Running
		};

		/**
		 * @brief Initialize all engine systems that don't require graphics or windowing
		 *
		 * The engine must be in the Dead state when this method is called
		 */
		void CoreInit(const InitConfig& initCfg);

		/**
		 * @brief Initialize the graphics backend and windowing system
		 *
		 * The engine must be in the Alive state when this method is called
		 */
		void GfxInit();

		/**
		 * @brief Perform any final pre-run tasks and start the game loop
		 *
		 * The engine must be in the Ready state when this method is called
		 */
		void Run();

		/**
		 * @brief Stop the game loop
		 *
		 * The engine must be in the Running state when this method is called
		 */
		void Quit();

		/**
		 * @brief Shutdown windowing and graphics systems
		 *
		 * @details Any GPU resources still existing at this time will be destroyed via appropriate functions before shutdown
		 *
		 * The engine must be in the Ready state when this method is called
		 */
		void GfxShutdown();

		/**
		 * @brief Shutdown all engine systems that don't require graphics or windowing
		 *
		 * @details After this function is called, the process may be safely exited or the engine may be re-initialized
		 *
		 * The engine must be in the Alive state when this method is called
		 */
		void CoreShutdown();

		/**
		 * @brief Get the current state of the engine
		 *
		 * @return The engine state
		 */
		State GetState() {
			std::lock_guard lk(stateMtx);
			return state;
		}

	  private:
		InitConfig icfg;
		RuntimeConfig rcfg;
		State state;
		std::mutex stateMtx;

		std::queue<std::function<void()>> mainThreadTasksQueue;
		std::mutex mttQueueMtx;
		std::thread::id mainThread;

		std::shared_ptr<exathread::Pool> pool;

		void RegisterBuiltinComponents();

		Engine();
		~Engine();
	};
}