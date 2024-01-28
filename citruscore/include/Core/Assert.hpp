#pragma once

#include <string>

namespace Citrus {
    class Asserts {
    public:
        static void EngineAssert(bool condition, std::string errorMsg);
        static void ClientAssert(bool condition, std::string errorMsg);  
    };
}