#pragma once

#include <string>
#include <memory>

#include "DllHelper.hpp"

namespace Cacao {

	/**
	 * @brief Logging manager singleton
	 */
	class CACAO_API LogMgr {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static LogMgr& Get() {
			static LogMgr _instance;
			return _instance;
		}

		///@cond
		LogMgr(const LogMgr&) = delete;
		LogMgr(LogMgr&&) = delete;
		LogMgr& operator=(const LogMgr&) = delete;
		LogMgr& operator=(LogMgr&&) = delete;
		///@endcond

		/**
		 * @brief Level of severity for log messages
		 */
		enum class Level {
			Trace = 0,///<Debug information, usually unneeded
			Info = 1, ///<Information
			Warn = 2, ///<Warnings
			Error = 3,///<Errors
			Fatal = 4 ///<Fatal Errors
		};

		/**
		 * @brief Log a message from the engine
		 *
		 * @param message The message to log
		 * @param level The severity of the message (optional, defaults to Info)
		 *
		 * @note For use only by the engine internally.
		 */
		void EngineLog(std::string message, Level level = Level::Info);

		/**
		 * @brief Log a message from the game
		 *
		 * @param message The message to log
		 * @param level The severity of the message (optional, defaults to Info)
		 *
		 * @note For use by games using the engine.
		 */
		void ClientLog(std::string message, Level level = Level::Info);

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		LogMgr();
		~LogMgr();
	};
}