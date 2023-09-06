#pragma once

#include "imgui.h"

namespace CitrusEngine {
    //Citrus Engine ImGui wrapper
    class ImGuiWrapper {
    public:
        //Initialize the wrapper
        static void Init();
        //Shutdown the wrapper
        static void Shutdown();

        //Create a new ImGui frame
        static void CreateFrame();
        //Compose an ImGui frame (render commands to draw data)
        static void ComposeFrame();
        //Render ImGui frame on-screen
        static void RenderFrame();

        
    private:
        static bool initialized;
        static bool frameCreated;
        static bool frameComposed;

        //Implemented by the backend

        //Initialize ImGui's backend implementation
        static bool InitImGuiBackend();
        //Shutdown ImGui's backend implementation
        static void ShutdownImGuiBackend();
        //Create a new frame using ImGui's backend implementation
        static void ImGuiBackendNewFrame();
        //Draw ImGui frame using ImGui's backend implementation
        static void ImGuiBackendRender(ImDrawData* drawData);
        //Run any backend-specific post-viewport rendering code
        static void ImGuiBackendPostViewport();
    };
}