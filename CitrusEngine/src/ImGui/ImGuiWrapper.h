#pragma once

#include "imgui.h"

#define NativeWindowType GLFWwindow

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