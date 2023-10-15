#pragma once

#include <string>

namespace CitrusEngine {
    class Asserts {
    public:
        static void EngineAssert(bool condition, std::string errorMsg);
        static void ClientAssert(bool condition, std::string errorMsg);  
    };
}