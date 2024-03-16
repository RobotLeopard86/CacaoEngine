#include "Core/Assert.hpp"
#include "Core/Log.hpp"

#include <stdlib.h>

namespace Cacao {
    void EngineAssert(bool condition, std::string errorMsg){
        if(!condition){
            Logging::EngineLog("Failed Assertion - " + errorMsg, LogLevel::Error);
            exit(-1);
        }
    }
    void ClientAssert(bool condition, std::string errorMsg){
        if(!condition){
            Logging::ClientLog("Failed Assertion - " + errorMsg, LogLevel::Error);
            exit(-1);
        }
    }
}