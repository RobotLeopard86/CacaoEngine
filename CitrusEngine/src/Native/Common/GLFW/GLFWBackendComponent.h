#pragma once

#include "GLFW/glfw3.h"

namespace CitrusEngine {

    //GLFW version of Backend.h (does not implement Backend.h symbols)
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