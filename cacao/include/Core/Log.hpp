#pragma once

#include "spdlog/logger.h"

#include <string>

namespace Cacao {
	enum LogLevel {
		Trace = 0,//Debug information, usually unneeded (e.g. renderer backend info)
		Info = 1, //Information (e.g. engine starting)
		Warn = 2, //Warnings (e.g. slow loading)
		Error = 3,//Errors (e.g. shader compilation failure)
		Fatal = 4 //Fatal Errors (e.g. backend initialization failure)
	};

	class Logging {
	  public:
		//Initialize the logging system
		static void Init();

		//Engine logs
		static void EngineLog(std::string message, LogLevel level = LogLevel::Info);

		//Client logs
		static void ClientLog(std::string message, LogLevel level = LogLevel::Info);

	  private:
		static std::shared_ptr<spdlog::logger> engineStdout, engineLogfile, clientStdout, clientLogfile;
	};
}