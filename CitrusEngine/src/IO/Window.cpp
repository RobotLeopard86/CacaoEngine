#include "Window.h"

#ifdef CE_PLATFORM_LINUX
    #include "LinuxOnly/LinuxWindow.h"
    #define GenWindow new LinuxWindow
#endif

#include "Core/Assert.h"
#include "Events/EventSystem.h"
#include "Core/Utilities.h"
#include "Core/CitrusClient.h"

namespace CitrusEngine {
    //Make window instance null pointer by default
    Window* Window::instance = nullptr;

    void Window::Create(std::string title, int initialSizeX, int initialSizeY) {
        //Make sure we don't have an active window
        Asserts::EngineAssert(instance == nullptr, "Cannot create window while a window is already active!");

        //Create a window of the type defined by the platform
        instance = GenWindow(title, initialSizeX, initialSizeY);

        Logging::EngineLog(LogLevel::Info, "Window created!");
    }

    void Window::Destroy() {
        //Make sure we have an active window
        Asserts::EngineAssert(instance != nullptr, "Cannot destroy uninitialized or destroyed window!");

        //Free window instance
        delete instance;
        //Assign null pointer to prevent another attempted destruction
        instance = nullptr;

        Logging::EngineLog(LogLevel::Info, "Window destroyed!");
    }
}