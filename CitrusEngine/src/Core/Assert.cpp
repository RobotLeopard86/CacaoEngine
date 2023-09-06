#include "Core/Assert.h"
#include "Core/Log.h"

#include <stdlib.h>

namespace CitrusEngine {
    void Asserts::EngineAssert(bool condition, std::string errorMsg){
        if(!condition){
            Logging::EngineLog(LogLevel::Error, "Failed Assertion - " + errorMsg);
            exit(-1);
        }
    }
    void Asserts::ClientAssert(bool condition, std::string errorMsg){
        if(!condition){
            Logging::ClientLog(LogLevel::Error, "Failed Assertion - " + errorMsg);
            exit(-1);
        }
    }
}