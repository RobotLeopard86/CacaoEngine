#pragma once

#include "Graphics/Window.h"

#include "GLFW/glfw3.h"

namespace CitrusEngine {

    //GNU/Linux implementation of Window (see Window.h for method details)
    class LinuxWindow : public Window {
    public:
        LinuxWindow(std::string title, int intialSizeX, int initialSizeY);

        void Update_Impl() override;
        void* GetNativeWindow_Impl() override;
        glm::i32vec2 GetSize_Impl() override;
        void SetSize_Impl(glm::i32vec2 newSize) override;
        void SetVSyncEnabled_Impl(bool value) override;
        bool IsVSyncEnabled_Impl() override;
        void Shutdown() override;
    private:
        bool useVSync;
        glm::i32vec2 size;
        std::string windowTitle;

        GLFWwindow* window;
    };
}