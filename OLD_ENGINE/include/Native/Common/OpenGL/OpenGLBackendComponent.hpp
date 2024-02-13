#pragma once

#include "glad/gl.h"

namespace CacaoEngine {

    //OpenGL version of Backend.hpp (does not implement Backend.hpp symbols)
    //Used to manage the OpenGL backend component
    class OpenGLBackendComponent {
    public:
        virtual ~OpenGLBackendComponent() {}

        static bool Initialize();
        static void Shutdown();

        static bool IsInitialized() { return initialized; }
    private:
        static bool initialized;
    };
}