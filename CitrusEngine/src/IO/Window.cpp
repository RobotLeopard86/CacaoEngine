#include "Window.h"

#include "Core/Assert.h"
#include "Events/EventSystem.h"
#include "Core/Utilities.h"
#include "Core/CitrusClient.h"

namespace CitrusEngine {
    //Make window instance null pointer by default
    Window* Window::instance = nullptr;

    void Window::Create(std::string title, int initialSizeX, int initialSizeY) {
        //Make sure we don't have an active window
        Asserts::EngineAssert(instance == nullptr, "Cannot create new window while a window is already active!");

        //Create a window of the type defined by the platform
        instance = CreateNativeWindow(title, initialSizeX, initialSizeY);

        Logging::EngineLog(LogLevel::Info, "Window created!");
    }

    void Window::Destroy() {
        //Make sure we have an active window
        Asserts::EngineAssert(instance != nullptr, "Cannot destroy uninitialized or destroyed window!");

        //Shutdown window
        instance->Shutdown();
        //Free window instance
        delete instance;
        //Assign null pointer to prevent another attempted destruction
        instance = nullptr;

        Logging::EngineLog(LogLevel::Info, "Window destroyed!");
    }

    void Window::Update(){
        //Make sure we have an active window
        Asserts::EngineAssert(instance != nullptr, "Cannot update uninitialized or destroyed window!");

        //Invoke native implementation
        instance->Update_Impl();
    }

    void* Window::GetNativeWindow(){
        //Make sure we have an active window
        Asserts::EngineAssert(instance != nullptr, "Cannot get native window for uninitialized or destroyed window!");

        //Invoke native implementation
        return instance->GetNativeWindow_Impl();
    }

    glm::i32vec2 Window::GetSize(){
        //Make sure we have an active window
        Asserts::EngineAssert(instance != nullptr, "Cannot get size of uninitialized or destroyed window!");

        //Invoke native implementation
        return instance->GetSize_Impl();
    }

    void Window::SetSize(glm::i32vec2 newSize){
        //Make sure we have an active window
        Asserts::EngineAssert(instance != nullptr, "Cannot get size of uninitialized or destroyed window!");

        //Invoke native implementation
        instance->SetSize_Impl(newSize);
    }

    void Window::SetVSyncEnabled(bool value){
        //Make sure we have an active window
        Asserts::EngineAssert(instance != nullptr, "Cannot enable or disable VSync for uninitialized or destroyed window!");

        //Invoke native implementation
        instance->SetVSyncEnabled_Impl(value);
    }

    bool Window::IsVSyncEnabled(){
        //Make sure we have an active window
        Asserts::EngineAssert(instance != nullptr, "Cannot get size of uninitialized or destroyed window!");

        //Invoke native implementation
        return instance->IsVSyncEnabled_Impl();
    }
}