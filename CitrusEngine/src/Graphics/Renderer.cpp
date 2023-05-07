#include "Renderer.h"

#include "Core/Log.h"
#include "Core/Assert.h"

namespace CitrusEngine {
    //Make renderer instance null pointer by default
    Renderer* Renderer::instance = nullptr;

    void Renderer::Create() {
        //Make sure we don't have an active renderer
        Asserts::EngineAssert(instance == nullptr, "Cannot create new renderer while one is already active!");

        //Create a renderer of the type defined at compile-time
        instance = CreateNativeRenderer();

        Logging::EngineLog(LogLevel::Info, "Renderer created!");
    }

    void Renderer::Shutdown() {
        //Make sure we have an active renderer
        Asserts::EngineAssert(instance != nullptr, "Cannot shut down uninitialized or already shutdown renderer!");

        //Free renderer instance
        delete instance;
        //Assign null pointer to prevent another attempted destruction
        instance = nullptr;

        Logging::EngineLog(LogLevel::Info, "Renderer shut down!");
    }

    void Renderer::SetClearColor(glm::u8vec3 color){
        //Make sure we have an active renderer
        Asserts::EngineAssert(instance != nullptr, "Cannot set clear color of uninitialized or shutdown renderer!");

        //Invoke native implementation
        instance->SetClearColor_Impl(color);
    }

    void Renderer::Clear(){
        //Make sure we have an active renderer
        Asserts::EngineAssert(instance != nullptr, "Cannot clear uninitialized or shutdown renderer!");

        //Invoke native implementation
        instance->Clear_Impl();
    }

    void Renderer::InitBackend(){
        //Make sure we have an active renderer
        Asserts::EngineAssert(instance != nullptr, "Cannot initialize backend for uninitialized or shutdown renderer!");

        //Invoke native implementation
        instance->InitBackend_Impl();
    }

    void Renderer::RenderGeometry(Mesh* mesh, Transform* transform, Shader* shader){
        //Make sure we have an active renderer
        Asserts::EngineAssert(instance != nullptr, "Cannot render geometry using uninitialized or shutdown renderer!");

        //Invoke native implementation
        instance->RenderGeometry_Impl(mesh, transform, shader);
    }

    void Renderer::ResizeViewport(int width, int height){
        //Make sure we have an active renderer
        Asserts::EngineAssert(instance != nullptr, "Cannot set viewport size using uninitialized or shutdown renderer!");

        //Invoke native implementation
        instance->ResizeViewport_Impl(width, height);
    }

    void Renderer::SetCamera(Camera* cam){
        //Make sure we have an active renderer
        Asserts::EngineAssert(instance != nullptr, "Cannot set camera for uninitialized or shutdown renderer!");

        //Invoke native implementation
        instance->SetCamera_Impl(cam);
    }
}