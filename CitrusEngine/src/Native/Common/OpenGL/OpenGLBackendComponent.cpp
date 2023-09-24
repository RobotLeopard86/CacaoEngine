#include "Native/Common/OpenGL/OpenGLBackendComponent.h"

#include "Core/Log.h"

namespace CitrusEngine {

    bool OpenGLBackendComponent::initialized = false;

    bool OpenGLBackendComponent::Initialize() {
        if(!initialized){
            //Initialize OpenGL
            int gladVersion = gladLoaderLoadGL();
            initialized = (gladVersion != 0);
            if(!initialized){
                Logging::EngineLog(LogLevel::Error, "Failed to initialize OpenGL!");
                return false;
            }
        } else {
            Logging::EngineLog(LogLevel::Warn, "Attempted initialization of OpenGL while initialized. Ignoring call.");
        }
        return initialized;
    }

    void OpenGLBackendComponent::Shutdown() {
        if(initialized){
            gladLoaderUnloadGL();
            initialized = false;
        } else {
            Logging::EngineLog(LogLevel::Warn, "Attempted shutdown of OpenGL while uninitialized. Ignoring call.");
        }
    }
}