#pragma once

#include "spdlog/spdlog.h"

#include <string>

namespace CitrusEngine {
    enum LogLevel {
        Trace=0, //Debug information, usually unneeded (e.g. render backend initialized)
        Info=1, //Information (e.g. engine starting)
        Warn=2, //Warnings (e.g. slow loading)
        Error=3, //Errors (e.g. shader compilation failure)
        Fatal=4 //Fatal Errors (e.g. memory leak)
    };

    class Logging {
    public:
        static void Setup();

        static void EngineLog(LogLevel level, std::string message);
        static void ClientLog(LogLevel level, std::string message);
    private:
        static std::shared_ptr<spdlog::logger> engineLogger;
        static std::shared_ptr<spdlog::logger> clientLogger;
    };
}