#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <format>

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

		/**
		 * @brief A small object used to make the logging API more ergonomic.
		 * 
		 * Use operator<< with it just like with std::cout to write log messages (no ending newline or @c std::endl necessary)
		 * You can also use LogFormatted just like you would @c std::format
		 */
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

			/**
			 * @brief Add a value to the log stream
			 * 
			 * @param value The value to log (ostream overrides for formatting like <tt>std::ostream& operator<<(std::ostream& out, const T& val)</tt>) should work
			 * 
			 * @return This object to continue the stream chain
			 */
			template<typename T>
			LogToken& operator<<(const T& value) {
				oss << value;
				return *this;
			}

			/**
			 * @brief Add a formatted string to the log stream
			 * 
			 * @param fmtstr The format string to log
			 * @param args The arguments to the format string
			 */
			template<typename... Args>
			void LogFormatted(std::format_string<Args...> fmtstr, Args&&... args) {
				oss << std::format(fmtstr, args...);
			}

			/**
			 * @brief Add a formatted string to the log stream with an explicit locale
			 * 
			 * @param locale The locale to use for formatting
			 * @param fmtstr The format string to log
			 * @param args The arguments to the format string
			 */
			template<typename... Args>
			void LogFormatted(const std::locale& locale, std::format_string<Args...> fmtstr, Args&&... args) {
				oss << std::format(locale, fmtstr, args...);
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
		 * @note For use only by the engine internally.
		 *
		 * @param level The severity of the message (optional, defaults to Info)
		 *
		 * @return A LogToken, which is streamed into like std::cout and logs the resulting message upon destruction. Do not store this return value.
		 */
		static LogToken Engine(Level level = Level::Info);

		/**
		 * @brief Log a message from the game
		 *
		 * @note For use by games using the engine.
		 *
		 * @param level The severity of the message (optional, defaults to Info)
		 *
		 * @return A LogToken, which is streamed into like std::cout and logs the resulting message upon destruction. Do not store this return value.
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