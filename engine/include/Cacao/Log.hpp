#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <fstream>

#include "DllHelper.hpp"

namespace Cacao {

	/**
	 * @brief Logging manager singleton
	 */
	class CACAO_API Logger {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static Logger& Get();

		///@cond
		Logger(const Logger&) = delete;
		Logger(Logger&&) = delete;
		Logger& operator=(const Logger&) = delete;
		Logger& operator=(Logger&&) = delete;
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

		///@brief A small object used to make the logging API more ergonomic. Use operator<< with it just like with std::cout to write log messages (no ending newline or std::endl necessary)
		struct LogToken {
			Level lvl;
			std::ostringstream oss;
			bool isClient;

		  public:
			LogToken() = default;
			LogToken(const LogToken&) = delete;
			LogToken(LogToken&&) = default;
			LogToken& operator=(const LogToken&) = delete;
			LogToken& operator=(LogToken&&) = default;

			template<typename T>
			LogToken& operator<<(const T& value) {
				oss << value;
				return *this;
			}

			~LogToken() {
				try {
					Logger::Get().ImplLog(oss.str(), lvl, isClient);
				} catch(...) {}
			}
		};

		/**
		 * @brief Log a message from the engine
		 *
		 * @param message The message to log
		 * @param level The severity of the message (optional, defaults to Info)
		 *
		 * @note For use only by the engine internally.
		 */
		static LogToken Engine(Level level = Level::Info);

		/**
		 * @brief Log a message from the game
		 *
		 * @param message The message to log
		 * @param level The severity of the message (optional, defaults to Info)
		 *
		 * @note For use by games using the engine.
		 */
		static LogToken Client(Level level = Level::Info);

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		void ImplLog(std::string message, Level level, bool isClient);

		friend LogToken;

		Logger();
		~Logger();
	};
}