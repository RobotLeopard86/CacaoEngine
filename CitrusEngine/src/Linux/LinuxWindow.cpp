#include "LinuxWindow.h"

#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/CitrusClient.h"

#include "Events/EventSystem.h"

#include "Graphics/Renderer.h"

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

        //Initialize backend
        Renderer::GetInstance()->InitBackend();

        //Enable VSync
        SetVSyncEnabled_Impl(true);

        //Set viewport size
        Renderer::GetInstance()->ResizeViewport(initialSizeX, initialSizeY);
    
        //Register GLFW callbacks
        
        //Called when window resized
        glfwSetWindowSizeCallback(window, [](GLFWwindow* glfwWindow, int sizeX, int sizeY){
            Window::SetSize(glm::i32vec2(sizeX, sizeY));
            WindowResizeEvent wre{sizeX, sizeY};
            CitrusClient::GetEventManager()->Dispatch(wre);
        });

        //Called when window closed
        glfwSetWindowCloseCallback(window, [](GLFWwindow* glfwWindow){
            WindowCloseEvent wce{};
            CitrusClient::GetEventManager()->Dispatch(wce);
        });

        //Called when window receives/loses focus
        glfwSetWindowFocusCallback(window, [](GLFWwindow* glfwWindow, int status){
            if(status == GLFW_TRUE){
                WindowReceiveFocusEvent wrfe = WindowReceiveFocusEvent{};
                CitrusClient::GetEventManager()->Dispatch(wrfe);
            } else if(status == GLFW_FALSE){
                WindowLoseFocusEvent wlfe = WindowLoseFocusEvent{};
                CitrusClient::GetEventManager()->Dispatch(wlfe);
            }
        });

        //Called when GLFW receives key input
        glfwSetKeyCallback(window, [](GLFWwindow* glfwWindow, int key, int scancode, int action, int mods){
            if(action == GLFW_PRESS || action == GLFW_REPEAT){
                KeyDownEvent kde = KeyDownEvent{key};
                CitrusClient::GetEventManager()->Dispatch(kde);
            } else if(action == GLFW_RELEASE){
                KeyUpEvent kue = KeyUpEvent{key};
                CitrusClient::GetEventManager()->Dispatch(kue);
            }
        });

        //Called when GLFW receives typing input
        glfwSetCharCallback(window, [](GLFWwindow* glfwWindow, unsigned int character){
            KeyTypeEvent kte = KeyTypeEvent{character};
            CitrusClient::GetEventManager()->Dispatch(kte);
        });

        //Called when GLFW receives mouse button input
        glfwSetMouseButtonCallback(window, [](GLFWwindow* glfwWindow, int btn, int action, int mods){
            if(action == GLFW_PRESS){
                MousePressEvent mpe = MousePressEvent{btn};
                CitrusClient::GetEventManager()->Dispatch(mpe);
            } else if(action == GLFW_RELEASE){
                MouseReleaseEvent mre = MouseReleaseEvent{btn};
                CitrusClient::GetEventManager()->Dispatch(mre);
            }
        });

        //Called when GLFW detects mouse movement
        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double mouseX, double mouseY){
            MouseMoveEvent mme = MouseMoveEvent{mouseX, mouseY};
            CitrusClient::GetEventManager()->Dispatch(mme);
        });

        //Called when GLFW detects mouse scroll input
        glfwSetScrollCallback(window, [](GLFWwindow*, double offsetX, double offsetY){
            MouseScrollEvent mse = MouseScrollEvent{offsetX, offsetY};
            CitrusClient::GetEventManager()->Dispatch(mse);
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

    void LinuxWindow::EnsureWindowRenderContext_Impl(){
        glfwMakeContextCurrent(window);
    }
}