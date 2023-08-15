#include "Core/Backend.h"

#include "Core/Assert.h"

#include "Native/Common/GLFW/GLFWBackendComponent.h"
#include "Graphics/Renderer.h"

namespace CitrusEngine {

    bool Backend::Initialize(){
        Asserts::EngineAssert(!initialized, "Backend already initialized!");

        //Initialize GLFW
        if(!GLFWBackendComponent::Initialize()){
            return false;
        }

        //Initialize OpenGL backend
        Renderer::GetInstance()->InitBackend();

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