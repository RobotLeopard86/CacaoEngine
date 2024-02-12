#pragma once

#include "GLFW/glfw3.h"

namespace CacaoEngine {

    //GLFW version of Backend.hpp (does not implement Backend.hpp symbols)
    //Used to manage the GLFW backend component
    class GLFWBackendComponent {
    public:
        virtual ~GLFWBackendComponent() {}

        static bool Initialize();
        static void Shutdown();

        static bool IsInitialized() { return initialized; }
    private:
        static bool initialized;
    };
}