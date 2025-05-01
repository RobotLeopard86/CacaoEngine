#pragma once

#include <atomic>
#include <string>
#include <map>
#include <queue>

#include "thread_pool/thread_pool.h"

#include "DllHelper.hpp"

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
		static Engine& Get() {
			static Engine _instance;
			return _instance;
		}

		/**
		 * @name Configuration
		 * Structs and methods related to engine configuration
		 */
		///@{

		/**
		 * @brief Configuration values for the engine
		 */
		struct CACAO_API Config {
			/**
			 * @brief The number of dynamic ticks that should occur within a second
			 * @details This is NOT a hard constraint. Attempting to run too many ticks within a second will cause noticeable frame skipping.
			 */
			int targetDynTPS;

			/**
			 * @brief The rate at which fixed ticks should occur
			 *
			 * @warning Fixed ticks are not yet implemented into the engine!
			 */
			int fixedTickRate;

			/**
			 * @brief The number of frames the renderer can be behind before skipping some to catch up
			 */
			int maxFrameLag;
		} config;

		/**
		 * @brief Configuration values that are set for startup and cannot be modified afterwards
		 */
		struct CACAO_API InitConfig {
			/**
			 * @brief Whether the engine is running outside of a bundle
			 *
			 * @details If this flag is set, the engine will not attempt to validate that it is in a game bundle on startup.
			 * This will also disable all systems related to the default asset loader and bundle systems.
			 * Assets, worlds, and game modules will need to be manually loaded and configured in this mode.
			 */
			bool standalone;

			/**
			 * @brief The requested backend to try to initialize first, overriding the default setting.
			 */
			std::string initialRequestedBackend;

			/**
			 * @brief (Linux only) Whether to disable attempting to use Wayland and force the usage of X11 instead
			 *
			 * @note This flag is ignored if not on Linux
			 */
			bool forceX11;
		};

		/**
		 * @brief Get an immutable reference to the initialization config values
		 *
		 * @return The initialization config values
		 */
		const InitConfig& GetInitConfig() {
			return cfg;
		}

		///@}

		/**
		 * @name Lifecycle
		 * Methods and data related to the engine lifecycle
		 */
		///@{

		/**
		 * @brief The current state of the engine
		 */
		enum class CACAO_API State {
			Dead,
			Alive,
			Stopped,
			Running
		};

		/**
		 * @brief Initialize all engine systems that don't require graphics or windowing
		 *
		 * @details If the engine is not in standalone mode, this will also trigger bundle loading.
		 *
		 * The engine must be in the Dead state when this method is called
		 *
		 * @param initCfg The initialization configuration
		 *
		 * @return Whether initialization succeeded
		 */
		bool CoreInit(const InitConfig& initCfg);

		/**
		 * @brief Initialize the graphics backend and windowing system
		 *
		 * The engine must be in the Alive state when this method is called
		 *
		 * @return Whether initialization succeeded
		 */
		bool GfxInit();

		/**
		 * @brief Perform any final pre-run tasks and start the game loop
		 *
		 * The engine must be in the Stopped state when this method is called
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
		 * The engine must be in the Stopped state when this method is called
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
		const State GetState() {
			return state;
		}

		///@}

	  private:
		InitConfig cfg;
		State state;
	};
}