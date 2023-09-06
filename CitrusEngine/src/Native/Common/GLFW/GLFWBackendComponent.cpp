#include "Native/Common/GLFW/GLFWBackendComponent.h"

#include "Core/Log.h"

namespace CitrusEngine {

    bool GLFWBackendComponent::initialized = false;

    bool GLFWBackendComponent::Initialize() {
        if(!initialized){
            //Initialize GLFW
            initialized = glfwInit();
            if(!initialized){
                Logging::EngineLog(LogLevel::Error, "Failed to initialize GLFW!");
                return false;
            }
            
            //Register an error callback with GLFW
            glfwSetErrorCallback([](int errCode, const char* description) {
                Logging::EngineLog(LogLevel::Error, "GLFW encountered an error with code " + std::to_string(errCode) + ", with provided reason \"" + description + "\"!");
            });
        } else {
            Logging::EngineLog(LogLevel::Warn, "Attempted initialization of GLFW while initialized. Ignoring call.");
        }
        return initialized;
    }

    void GLFWBackendComponent::Shutdown() {
        if(initialized){
            glfwTerminate();
            initialized = false;
        } else {
            Logging::EngineLog(LogLevel::Warn, "Attempted shutdown of GLFW while uninitialized. Ignoring call.");
        }
    }
}