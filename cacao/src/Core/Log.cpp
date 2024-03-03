#include "Core/Log.hpp"

#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#include <filesystem>

namespace Cacao {

    //Required definitions of static members
    std::shared_ptr<spdlog::logger> Logging::engineLogger = nullptr;
    std::shared_ptr<spdlog::logger> Logging::clientLogger = nullptr;

    void Logging::Init(){
        //Create logging "sinks" (outputs)
        std::vector<spdlog::sink_ptr> sinks;

        //Standard output color sink
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        //Get logfile path
        std::string logfilepath = std::filesystem::current_path().string() + "/cacao-engine.log";

        //File output sink
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfilepath, true));

        //Create engine and client loggers that write to both defined sinks
        engineLogger = std::make_shared<spdlog::logger>("cacaoengine", begin(sinks), end(sinks));
        clientLogger = std::make_shared<spdlog::logger>("cacaoclient", begin(sinks), end(sinks));

        //Register loggers with spdlog so that they use the defined pattern
        spdlog::register_logger(engineLogger);
        spdlog::register_logger(clientLogger);

        //Apply logging pattern (Month/Day/Year - Hour:Minute:Second AM/PM [Logger Name/Message Level]: Message Text)
        spdlog::set_pattern("%^%m/%d/%Y - %I:%M:%S %p [%n/%l]: %v%$");

        //Force log file flushing
        spdlog::flush_on(spdlog::level::trace);

        engineLogger->set_level(spdlog::level::trace);
        clientLogger->set_level(spdlog::level::trace);
    }

    void Logging::EngineLog(std::string message, LogLevel level){
        //Make sure that logging is setup
        if(engineLogger == nullptr){
            //If not, set logging up
            Init();
        }

        //Send a different log level based on tre one specified
        switch(level){
        case 0:
            engineLogger->trace(message);
            break;
        case 1:
            engineLogger->info(message);
            break;
        case 2:
            engineLogger->warn(message);
            break;
        case 3:
            engineLogger->error(message);
            break;
        case 4:
            engineLogger->critical(message);
            break;
        }
    }

    void Logging::ClientLog(std::string message, LogLevel level){
        //Make sure that logging is setup
        if(clientLogger == nullptr){
            //If not, set logging up
            Init();
        }

        //Send a different log level based on tre one specified
        switch(level){
        case 0:
            clientLogger->trace(message);
            break;
        case 1:
            clientLogger->info(message);
            break;
        case 2:
            clientLogger->warn(message);
            break;
        case 3:
            clientLogger->error(message);
            break;
        case 4:
            clientLogger->critical(message);
            break;
        }
    }

}