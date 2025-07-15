#include "Cacao/Log.hpp"
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
	};

	Logger::Logger() {
		//Get logfile path
		std::filesystem::path logfilePath = std::filesystem::current_path() / "cacao.log";

		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//Create sinks
		impl->logfileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfilePath.string(), true);
		impl->stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>(spdlog::color_mode::always);
		std::array<spdlog::sink_ptr, 2> sinks {{impl->logfileSink, impl->stdoutSink}};

		//Create and register loggers
		impl->engine = std::make_shared<spdlog::logger>("engine", sinks.begin(), sinks.end());
		impl->client = std::make_shared<spdlog::logger>("client", sinks.begin(), sinks.end());
		spdlog::register_logger(impl->engine);
		spdlog::register_logger(impl->client);
		spdlog::set_level(spdlog::level::trace);

		//Apply logging pattern (Month/Day/Year @ Hour:Minute:Second AM/PM [Logger Name:Thread ID/Message Level]: Message Text)
		spdlog::set_pattern("%^%m/%d/%Y @ %I:%M:%S %p [%n:%t/%l]: %v%$");

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