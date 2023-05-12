#pragma once

#include "imgui.h"

#ifdef CE_PLATFORM_LINUX
    #define NativeWindowType GLFWwindow
    #include "GLFW/glfw3.h"
    #include "imgui/backends/imgui_impl_glfw.h"
#endif

#ifdef CE_RENDERER_GL
    #include "glad/gl.h"
    #include "imgui/backends/imgui_impl_opengl3.h"
#endif

namespace CitrusEngine {
    //Citrus Engine ImGui wrapper

    /* This code uses defined macro values. Here is an explanation list:
    NativeWindowType - The type used by the platform for a window (e.g. GLFWwindow)
    NativePlatform - The name of the platform windowing API for ImGui (e.g. Glfw)
    NativeRenderingAPI - The native rendering API for ImGui (e.g. OpenGL)
    NativeRenderingSuffix - An optional value representing a suffix for additional data (e.g. 3 for OpenGL 3)
    NativeRenderingArgs - Any arguments necessary for ImGui to initialize for the correct rendering API (e.g. "#version 410" to specify OpenGL 4.1)
    */
    class ImGuiWrapper {
    public:
        static void Init();
        static void Shutdown();
        static void FrameSetup();
        static void FrameRender();
    private:
        static bool initialized;
        static bool renderingOK;
        static NativeWindowType* nativeWindow;
    };
}