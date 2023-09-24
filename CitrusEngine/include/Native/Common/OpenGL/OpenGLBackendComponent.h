#pragma once

#include "glad/gl.h"

namespace CitrusEngine {

    //OpenGL version of Backend.h (does not implement Backend.h symbols)
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