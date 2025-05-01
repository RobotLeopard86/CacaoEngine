#include "Cacao/Log.hpp"

#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#include <filesystem>

namespace Cacao {

	//Impl struct
	struct LogMgr::Impl {
		std::shared_ptr<spdlog::logger> engine, client;
		std::shared_ptr<spdlog::sinks::basic_file_sink_mt> logfileSink;
		std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> stdoutSink;
		void Log(std::string message, Level level, bool isClient);
	};

	LogMgr::LogMgr() {
		//Get logfile path
		std::filesystem::path logfilePath = std::filesystem::current_path() / "cacao.log";

		//Create sinks
		impl->logfileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfilePath.string(), false);
		impl->stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>(spdlog::color_mode::always);
		std::array<spdlog::sink_ptr, 2> sinks {{impl->logfileSink, impl->stdoutSink}};

		//Create implementation pointer
		impl = std::make_unique<Impl>();

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

	LogMgr::~LogMgr() {}

	void LogMgr::EngineLog(std::string message, Level level) {
		impl->Log(message, level, false);
	}

	void LogMgr::ClientLog(std::string message, Level level) {
		impl->Log(message, level, true);
	}

	void LogMgr::Impl::Log(std::string message, Level level, bool isClient) {
		//Set logger
		std::shared_ptr<spdlog::logger> logger = (isClient ? client : engine);

		//Make sure the logger is registered
		spdlog::register_logger(logger);

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
				engine->error(message);
				break;
			case Level::Fatal:
				logger->critical(message);
				break;
		}
	}

}