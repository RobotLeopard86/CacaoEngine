#include "Graphics/Window.hpp"

#include "Core/Log.hpp"
#include "Core/Assert.hpp"
#include "Core/Backend.hpp"

#include "Events/EventSystem.hpp"

#include "Native/Common/GLFW/GLFWBackendComponent.hpp"

//GLFW implementation of Window (see Window.hpp for more details)

//CE_GLFW_API should be set by the backend Makefile compiling this file
#ifndef CE_GLFW_API
    #error "You must define CE_GLFW_API when compiling this file! Visit https://www.glfw.org/docs/latest/window_guide.html#GLFW_CLIENT_API_hint to see the options."
#endif

namespace CacaoEngine {

    std::map<void*, Window*> Window::nativeWindowLUT = std::map<void*, Window*>();

    Window* Window::Create(std::string title, int initialSizeX, int initialSizeY){
        if(!GLFWBackendComponent::IsInitialized()){
            Logging::EngineLog(LogLevel::Error, "Attempted window creation while GLFW backend component is uninitialized. Null pointer will be returned.");
            return nullptr;
        }

        //GLFW setup
        glfwWindowHint(GLFW_CLIENT_API, CE_GLFW_API);
        GLFWwindow* nativeWindow = glfwCreateWindow(initialSizeX, initialSizeY, title.c_str(), 0, 0);

        if(glfwGetWindowAttrib(nativeWindow, GLFW_CLIENT_API) == GLFW_OPENGL_API) glfwMakeContextCurrent(nativeWindow);

        //Initialize rendering backend
        Asserts::EngineAssert(Backend::InitRenderer(), "Renderer failed to initialize!");

        //Create window object
        Window* window = new Window((void*)nativeWindow, glm::ivec2(initialSizeX, initialSizeY), title);
        nativeWindowLUT.insert_or_assign((void*)nativeWindow, window);

        //Register GLFW callbacks
        
        //Called when window resized
        glfwSetWindowSizeCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* glfwWindow, int sizeX, int sizeY){
            Window::nativeWindowLUT.at((void*)glfwWindow)->SetSize({ sizeX, sizeY });
            WindowResizeEvent wre{sizeX, sizeY};
            EventManager::GetInstance()->Dispatch(wre);
        });

        //Called when window closed
        glfwSetWindowCloseCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* glfwWindow){
            WindowCloseEvent wce{};
            EventManager::GetInstance()->Dispatch(wce);
        });

        //Called when window receives/loses focus
        glfwSetWindowFocusCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* glfwWindow, int status){
            if(status == GLFW_TRUE){
                WindowReceiveFocusEvent wrfe = WindowReceiveFocusEvent{};
                EventManager::GetInstance()->Dispatch(wrfe);
            } else if(status == GLFW_FALSE){
                WindowLoseFocusEvent wlfe = WindowLoseFocusEvent{};
                EventManager::GetInstance()->Dispatch(wlfe);
            }
        });

        //Called when GLFW receives key input
        glfwSetKeyCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* glfwWindow, int key, int scancode, int action, int mods){
            if(action == GLFW_PRESS || action == GLFW_REPEAT){
                KeyDownEvent kde = KeyDownEvent{key};
                EventManager::GetInstance()->Dispatch(kde);
            } else if(action == GLFW_RELEASE){
                KeyUpEvent kue = KeyUpEvent{key};
                EventManager::GetInstance()->Dispatch(kue);
            }
        });

        //Called when GLFW receives typing input
        glfwSetCharCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* glfwWindow, unsigned int character){
            KeyTypeEvent kte = KeyTypeEvent{character};
            EventManager::GetInstance()->Dispatch(kte);
        });

        //Called when GLFW receives mouse button input
        glfwSetMouseButtonCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* glfwWindow, int btn, int action, int mods){
            if(action == GLFW_PRESS){
                MousePressEvent mpe = MousePressEvent{btn};
                EventManager::GetInstance()->Dispatch(mpe);
            } else if(action == GLFW_RELEASE){
                MouseReleaseEvent mre = MouseReleaseEvent{btn};
                EventManager::GetInstance()->Dispatch(mre);
            }
        });

        //Called when GLFW detects mouse movement
        glfwSetCursorPosCallback((GLFWwindow*)nativeWindow, [](GLFWwindow* window, double mouseX, double mouseY){
            MouseMoveEvent mme = MouseMoveEvent{mouseX, mouseY};
            EventManager::GetInstance()->Dispatch(mme);
        });

        //Called when GLFW detects mouse scroll input
        glfwSetScrollCallback((GLFWwindow*)nativeWindow, [](GLFWwindow*, double offsetX, double offsetY){
            MouseScrollEvent mse = MouseScrollEvent{offsetX, offsetY};
            EventManager::GetInstance()->Dispatch(mse);
        });

        return window;
    }

    void Window::UpdateVSyncState(){
        glfwSwapInterval(useVSync);
    }

    void Window::UpdateWindowSize(){
        glfwSetWindowSize((GLFWwindow*)nativeWindow, size.x, size.y);
    }

    void Window::Update(){
        glfwPollEvents();
        glfwSwapBuffers((GLFWwindow*)nativeWindow);
    }

    void Window::Destroy(){
        nativeWindowLUT.erase(nativeWindow);
        glfwDestroyWindow((GLFWwindow*)nativeWindow);
        delete this;
    }
}