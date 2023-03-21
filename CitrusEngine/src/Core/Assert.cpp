#include "Assert.h"
#include "Log.h"

#include <stdlib.h>

namespace CitrusEngine {
    void Asserts::EngineAssert(bool condition, std::string errorMsg){
        #ifdef CE_ALLOWASSERTS
            if(!condition){
                Logging::EngineLog(LogLevel::Error, "Failed Assertion - " + errorMsg);
                exit(-1);
            }
        #endif
    }
    void Asserts::ClientAssert(bool condition, std::string errorMsg){
        #ifdef CE_ALLOWASSERTS
            if(!condition){
                Logging::ClientLog(LogLevel::Error, "Failed Assertion - " + errorMsg);
                exit(-1);
            }
        #endif
    }
}