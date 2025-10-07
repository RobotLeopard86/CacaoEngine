#include "Cacao/Log.hpp"
#include "Cacao/Engine.hpp"
#include "SingletonGet.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#include <filesystem>

namespace Cacao {

	//Impl struct
	struct Logger::Impl {
		std::shared_ptr<spdlog::logger> engine, client;
		std::shared_ptr<spdlog::sinks::basic_file_sink_mt> logfileSink;
		std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> stdoutSink;
		bool noLog;
	};

	Logger::Logger() {
		//Get logfile path
		std::filesystem::path logfilePath = std::filesystem::current_path() / "cacao.log";

		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//Create sinks
		std::vector<spdlog::sink_ptr> sinks;
		if(!Engine::Get().GetInitConfig().suppressFileLogging) {
			impl->logfileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfilePath.string(), true);
			sinks.push_back(impl->logfileSink);
		}
		if(!Engine::Get().GetInitConfig().suppressConsoleLogging) {
			impl->stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>(spdlog::color_mode::always);
			sinks.push_back(impl->stdoutSink);
		}
		impl->noLog = sinks.size() <= 0;
		if(impl->noLog) return;

		//Create and register loggers
		impl->engine = std::make_shared<spdlog::logger>("engine", sinks.begin(), sinks.end());
		impl->client = std::make_shared<spdlog::logger>("client", sinks.begin(), sinks.end());
		spdlog::register_logger(impl->engine);
		spdlog::register_logger(impl->client);
		spdlog::set_level(spdlog::level::trace);

		//Apply logging pattern (Month/Day/Year @ Hour:Minute:Second [Logger Name:Thread ID/Message Level]: Message Text)
		spdlog::set_pattern("%^%D @ %X [%n:%t/%l]: %v%$");

		//Force log file flushing
		spdlog::flush_on(spdlog::level::trace);
	}

	Logger::~Logger() {}

	CACAOST_GET(Logger)

	Logger::LogToken Logger::Engine(Level level) {
		LogToken lt;
		lt.isClient = false;
		lt.lvl = level;
		return lt;
	}

	Logger::LogToken Logger::Client(Level level) {
		LogToken lt;
		lt.isClient = true;
		lt.lvl = level;
		return lt;
	}

	void Logger::ImplLog(std::string message, Level level, bool isClient) {
		//Skip if no logging enabled
		if(impl->noLog) return;

		//Set logger
		std::shared_ptr<spdlog::logger> logger = (isClient ? impl->client : impl->engine);

		//Send a log message using the level specified
		switch(level) {
			case Level::Trace:
				logger->trace(message);
				break;
			case Level::Info:
				logger->info(message);
				break;
			case Level::Warn:
				logger->warn(message);
				break;
			case Level::Error:
				logger->error(message);
				break;
			case Level::Fatal:
				logger->critical(message);
				break;
		}
	}

}