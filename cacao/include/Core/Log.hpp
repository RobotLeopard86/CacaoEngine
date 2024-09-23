#pragma once

#include "spdlog/logger.h"

#include <string>

namespace Cacao {

	/**
	 * @brief Level of severity for log messages
	 */
	enum LogLevel {
		Trace = 0,///<Debug information, usually unneeded
		Info = 1, ///<Information
		Warn = 2, ///<Warnings
		Error = 3,///<Errors
		Fatal = 4 ///<Fatal Errors
	};

	/**
	 * @brief Logging manager
	 */
	class Logging {
	  public:
		/**
		 * @brief Initialize the logging system
		 *
		 * @note This function is called by at engine startup automatically.
		 */
		static void Init();

		/**
		 * @brief Log a message from the engine
		 *
		 * @param message The message to log
		 * @param level The @ref LogLevel to use. (optional, defaults to Info)
		 *
		 * @note For use only by the engine internally.
		 */
		static void EngineLog(std::string message, LogLevel level = LogLevel::Info);

		/**
		 * @brief Log a message from the game
		 *
		 * @param message The message to log
		 * @param level The @ref LogLevel to use. (optional, defaults to Info)
		 *
		 * @note For use by games using the engine.
		 */
		static void ClientLog(std::string message, LogLevel level = LogLevel::Info);

	  private:
		static std::shared_ptr<spdlog::logger> engineStdout, engineLogfile, clientStdout, clientLogfile;
	};
}