#include "Core/Backend.h"

#include "Core/Assert.h"

#include "Native/Common/GLFW/GLFWBackendComponent.h"
#include "Native/Common/OpenGL/OpenGLBackendComponent.h"

namespace CitrusEngine {

    //Neither backend nor renderer are initialized by default
    bool Backend::initialized = false;
    bool Backend::rendererInitialized = false;

    bool Backend::Initialize(){
        Asserts::EngineAssert(!initialized, "Backend already initialized!");

        //Initialize GLFW
        if(!GLFWBackendComponent::Initialize()){
            return false;
        }

        //OpenGL must be initialized after window creation

        initialized = true;
        return true;
    }

    bool Backend::InitRenderer(){
        Asserts::EngineAssert(initialized, "Backend must be initialized before initializing renderer!");
        Asserts::EngineAssert(!rendererInitialized, "Renderer already initialized!");

        //Initialize OpenGL
        if(!OpenGLBackendComponent::Initialize()){
            return false;
        }

        rendererInitialized = true;
        return true;
    }

    void Backend::Shutdown(){
        Asserts::EngineAssert(initialized, "Backend isn't initialized!");

        //Shutdown GLFW
        GLFWBackendComponent::Shutdown();

        initialized = false;
    }

    void Backend::ShutdownRenderer(){
        Asserts::EngineAssert(initialized, "Backend must be initialized while shutting down renderer!");
        Asserts::EngineAssert(rendererInitialized, "Renderer is not already initialized!");

        //Shutdown OpenGL
        OpenGLBackendComponent::Shutdown();
        
        rendererInitialized = false;
    }
}