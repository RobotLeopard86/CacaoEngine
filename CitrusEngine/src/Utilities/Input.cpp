#include "Input.h"

#include "Core/Assert.h"
#include "Core/Log.h"

namespace CitrusEngine {
    //Make input system instance null pointer by default
    Input* Input::instance = nullptr;

    void Input::Create() {
        //Make sure we don't have an active instance
        Asserts::EngineAssert(instance == nullptr, "Cannot create new input system instance while one is already active!");

        //Create an input system of the type defined by the platform
        instance = CreateNativeInput();

        Logging::EngineLog(LogLevel::Info, "Input system instance created!");
    }

    void Input::Shutdown() {
        //Make sure we have an active instance
        Asserts::EngineAssert(instance != nullptr, "Cannot shut down uninitialized or destroyed input system instance!");

        //Shutdown window
        instance->Shutdown();
        //Free input system instance
        delete instance;
        //Assign null pointer to prevent another attempted destruction
        instance = nullptr;

        Logging::EngineLog(LogLevel::Info, "Input system instance shut down!");
    }

    bool Input::IsKeyPressed(int key) {
        //Make sure we have an active instance
        Asserts::EngineAssert(instance != nullptr, "Cannot check key press state using uninitialized or destroyed input system instance!");

        return instance->IsKeyPressed_Impl(key);
    }

    bool Input::IsMouseButtonPressed(int button) {
        //Make sure we have an active instance
        Asserts::EngineAssert(instance != nullptr, "Cannot check mouse button press state using uninitialized or destroyed input system instance!");

        return instance->IsMouseButtonPressed_Impl(button);
    }

    glm::dvec2 Input::GetCursorPos() {
        //Make sure we have an active instance
        Asserts::EngineAssert(instance != nullptr, "Cannot get cursor position using uninitialized or destroyed input system instance!");

        return instance->GetCursorPos_Impl();
    }
}