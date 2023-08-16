#include "Core/Backend.h"

#include "Core/Assert.h"

#include "Graphics/Renderer.h"

#include "Native/Common/GLFW/GLFWBackendComponent.h"

namespace CitrusEngine {

    bool Backend::initialized = false;

    bool Backend::Initialize(){
        Asserts::EngineAssert(!initialized, "Backend already initialized!");

        //Initialize GLFW
        if(!GLFWBackendComponent::Initialize()){
            return false;
        }

        //Glad must be initialized after window creation

        return true;
    }

    void Backend::Shutdown(){
        Asserts::EngineAssert(initialized, "Backend isn't initialized!");

        //Shutdown OpenGL backend
        Renderer::GetInstance()->ShutdownBackend();

        //Shutdown GLFW
        GLFWBackendComponent::Shutdown();
    }
}