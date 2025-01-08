#include "Core/Log.hpp"

#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"

#include <filesystem>

namespace Cacao {

	//Required definitions of static members
	std::shared_ptr<spdlog::logger> Logging::engineStdout = nullptr;
	std::shared_ptr<spdlog::logger> Logging::engineLogfile = nullptr;
	std::shared_ptr<spdlog::logger> Logging::clientStdout = nullptr;
	std::shared_ptr<spdlog::logger> Logging::clientLogfile = nullptr;
	std::shared_ptr<spdlog::logger> Logging::rtStdout = nullptr;
	std::shared_ptr<spdlog::logger> Logging::rtLogfile = nullptr;

	void Logging::Init() {
		//Standard color output loggers
		engineStdout = spdlog::stdout_color_mt("engine");
		clientStdout = spdlog::stdout_color_mt("client");
		rtStdout = spdlog::stdout_color_mt("runtime");

		//Get logfile path
		std::string logfilepath = std::filesystem::current_path().string() + "/cacao-engine.log";

		//Logfile loggers
		engineLogfile = spdlog::basic_logger_mt("engine", logfilepath, true);
		clientLogfile = spdlog::basic_logger_mt("client", logfilepath, true);
		rtLogfile = spdlog::basic_logger_mt("runtime", logfilepath, true);

		//Apply logging pattern (Month/Day/Year @ Hour:Minute:Second AM/PM [Logger Name:Thread ID/Message Level]: Message Text)
		spdlog::set_pattern("%^%m/%d/%Y @ %I:%M:%S %p [%n:%t/%l]: %v%$");

		//Force log file flushing
		spdlog::flush_on(spdlog::level::trace);

		engineStdout->set_level(spdlog::level::trace);
		engineLogfile->set_level(spdlog::level::trace);
		clientStdout->set_level(spdlog::level::trace);
		clientLogfile->set_level(spdlog::level::trace);
		rtStdout->set_level(spdlog::level::trace);
		rtLogfile->set_level(spdlog::level::trace);
	}

	void Logging::EngineLog(std::string message, LogLevel level) {
		//Make sure that logging is setup
		if(engineStdout == nullptr || engineLogfile == nullptr) {
			//If not, set logging up
			Init();
		}

		//Send a log message using the level specified
		switch(level) {
			case 0://Trace
				engineStdout->trace(message);
				engineLogfile->trace(message);
				break;
			case 1://Info
				engineStdout->info(message);
				engineLogfile->info(message);
				break;
			case 2://Warning
				engineStdout->warn(message);
				engineLogfile->warn(message);
				break;
			case 3://Error
				engineStdout->error(message);
				engineLogfile->error(message);
				break;
			case 4://Fatal Error
				engineStdout->critical(message);
				engineLogfile->critical(message);
				break;
		}
	}

	void Logging::RuntimeLog(std::string message, LogLevel level) {
		//Make sure that logging is setup
		if(rtStdout == nullptr || rtLogfile == nullptr) {
			//If not, set logging up
			Init();
		}

		//Send a log message using the level specified
		switch(level) {
			case 0://Trace
				rtStdout->trace(message);
				rtLogfile->trace(message);
				break;
			case 1://Info
				rtStdout->info(message);
				rtLogfile->info(message);
				break;
			case 2://Warning
				rtStdout->warn(message);
				rtLogfile->warn(message);
				break;
			case 3://Error
				rtStdout->error(message);
				rtLogfile->error(message);
				break;
			case 4://Fatal Error
				rtStdout->critical(message);
				rtLogfile->critical(message);
				break;
		}
	}

	void Logging::ClientLog(std::string message, LogLevel level) {
		//Make sure that logging is setup
		if(clientStdout == nullptr || clientLogfile == nullptr) {
			//If not, set logging up
			Init();
		}

		//Send a log message using the level specified
		switch(level) {
			case 0://Trace
				clientStdout->trace(message);
				clientLogfile->trace(message);
				break;
			case 1://Info
				clientStdout->info(message);
				clientLogfile->info(message);
				break;
			case 2://Warning
				clientStdout->warn(message);
				clientLogfile->warn(message);
				break;
			case 3://Error
				clientStdout->error(message);
				clientLogfile->error(message);
				break;
			case 4://Fatal Error
				clientStdout->critical(message);
				clientLogfile->critical(message);
				break;
		}
	}

}