#pragma once

#include "imgui.h"

#define NativeWindowType GLFWwindow
#include "GLFW/glfw3.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "glad/gl.h"
#include "imgui/backends/imgui_impl_opengl3.h"

namespace CitrusEngine {
    //Citrus Engine ImGui wrapper

    /* This code uses the defined macro value NativeWindowType, which is the native type used by the platform for a window (e.g. GLFWwindow)
    */
    class ImGuiWrapper {
    public:
        static void Init();
        static void Shutdown();
        static void CreateFrame();
        static void ComposeFrame();
        static void RenderFrame();
    private:
        static bool initialized;
        static bool frameCreated;
        static bool frameComposed;
        static NativeWindowType* nativeWindow;
    };
}