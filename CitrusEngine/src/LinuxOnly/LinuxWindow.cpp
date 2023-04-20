#include "LinuxWindow.h"

#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/CitrusClient.h"

#include "Events/EventSystem.h"

#include "glad/glad.h"

namespace CitrusEngine {

    LinuxWindow::LinuxWindow(std::string title, int initialSizeX, int initialSizeY)
        : windowTitle(title), size(glm::i32vec2(initialSizeX, initialSizeY)){
        
        //glfwInit returns a success value
        bool glfwSuccessfulInit = glfwInit();
        
        //Ensure GLFW initialized correctly
        Asserts::EngineAssert(glfwSuccessfulInit, "Failed to initialize GLFW!");

        //Register a callback to run when an error ocurrs in GLFW
        glfwSetErrorCallback([](int errCode, const char* description) {
            Logging::EngineLog(LogLevel::Error, "GLFW encountered an error with code " + std::to_string(errCode) + ", with provided reason \"" + description + "\"!");
        });

        //Create the window
        window = glfwCreateWindow(initialSizeX, initialSizeY, windowTitle.c_str(), nullptr, nullptr);
    
        //Create rendering context
        glfwMakeContextCurrent(window);

        //Enable VSync
        SetVSyncEnabled_Impl(true);

        //Initialize Glad (OpenGL loader)
        bool gladSuccessfulInit = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        //Ensure Glad initialized correctly
        Asserts::EngineAssert(gladSuccessfulInit, "Failed to initialize Glad!");

        //Log GL info
        const char* glVendor = (const char*)glGetString(GL_VENDOR);
        const char* glVersion = (const char*)glGetString(GL_VERSION);
        const char* glRenderer = (const char*)glGetString(GL_RENDERER);
        std::string msg = "Citrus Engine OpenGL Info:\n  OpenGL v";
        msg = msg + glVersion + " provided by " + glVendor + ", running on " + glRenderer;
        Logging::EngineLog(LogLevel::Trace, msg);
    
        //Register GLFW callbacks
        
        //Called when window resized
        glfwSetWindowSizeCallback(window, [](GLFWwindow* glfwWindow, int sizeX, int sizeY){
            Window::SetSize(glm::i32vec2(sizeX, sizeY));
            WindowResizeEvent wre{sizeX, sizeY};
            CitrusClient::GetEventManager()->Dispatch(wre);
        });

        //Called when window closwed
        glfwSetWindowCloseCallback(window, [](GLFWwindow* glfwWindow){
            WindowCloseEvent wce{};
            CitrusClient::GetEventManager()->Dispatch(wce);
        });
    }

    Window* Window::CreateNativeWindow(std::string title, int initialSizeX, int initialSizeY){
        return new LinuxWindow(title, initialSizeX, initialSizeY);
    }

    void LinuxWindow::Shutdown(){
        //Destroy the window
        glfwDestroyWindow(window);
        //Shutdown GLFW
        glfwTerminate();
    }

    void LinuxWindow::Update_Impl(){
        //Get events from GLFW
        glfwPollEvents();
        //Swap buffers
        glfwSwapBuffers(window);
    }

    void LinuxWindow::SetVSyncEnabled_Impl(bool value){
        if(value){
            //Wait for the monitor to update before swapping buffers (frame-rate capped at monitor refresh rate, but no screen tearing)
            glfwSwapInterval(1);
        } else {
            //Swap buffers immediately (higher frame-rate, but may cause screen tearing)
            glfwSwapInterval(0);
        }

        useVSync = value;
    }

    bool LinuxWindow::IsVSyncEnabled_Impl(){
        return useVSync;
    }

    void LinuxWindow::SetSize_Impl(glm::i32vec2 newSize){
        size = newSize;
    }

    glm::i32vec2 LinuxWindow::GetSize_Impl(){
        return size;
    }

    void* LinuxWindow::GetNativeWindow_Impl(){
        return window;
    }
}