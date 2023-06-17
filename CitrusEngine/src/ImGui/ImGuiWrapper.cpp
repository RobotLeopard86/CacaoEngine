#include "ImGuiWrapper.h"

#include "Core/Log.h"
#include "Core/Assert.h"

#include "Graphics/Window.h"

#include "glad/gl.h"

#include <stdexcept>

namespace CitrusEngine {

    //Initialize static variables
    bool ImGuiWrapper::renderingOK = false;
    bool ImGuiWrapper::initialized = false;
    bool ImGuiWrapper::drawingOK = false;
    NativeWindowType* ImGuiWrapper::nativeWindow = nullptr;

    //This code uses the defined macro NativeWindowType which is the type used by the platform for a window (e.g. GLFWwindow)
    void ImGuiWrapper::Init(){
        Asserts::EngineAssert(!initialized, "ImGui is already initialized!");
        //Create an ImGui context
		ImGui::CreateContext();

        //Obtain the native window type
        try {
            nativeWindow = static_cast<NativeWindowType*>(Window::GetNativeWindow());
        } catch(const std::bad_cast& exception){
            Logging::EngineLog(LogLevel::Error, "Could not obtain native window type, cancelling ImGui initialization!");
            return;
        }

        ImGuiIO& imGuiIO = ImGui::GetIO();

        //Apply ImGui custom style
        ImVec4* colors = ImGui::GetStyle().Colors;
        ImGui::GetStyle().FrameRounding = 5;
        ImGui::GetStyle().PopupRounding = 3;
        ImGui::GetStyle().WindowRounding = 4;
        ImGui::GetStyle().ChildRounding = 4;
        colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.03f, 0.26f, 0.00f, 0.54f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.13f, 0.73f, 0.24f, 0.40f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.03f, 0.62f, 0.26f, 0.67f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.48f, 0.27f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.15f, 0.15f, 0.15f, 0.40f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.12f, 0.12f, 0.12f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.02f, 0.53f, 0.27f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.13f, 0.75f, 0.29f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.11f, 0.70f, 0.28f, 0.40f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.03f, 0.62f, 0.28f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.02f, 0.34f, 0.17f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.17f, 0.17f, 0.17f, 0.31f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.07f, 0.57f, 0.28f, 0.80f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.00f, 0.31f, 0.08f, 1.00f);
        colors[ImGuiCol_Separator]              = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.00f, 0.00f, 0.00f, 0.78f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.22f, 0.22f, 0.22f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.42f, 0.42f, 0.42f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.02f, 0.32f, 0.17f, 0.86f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.00f, 0.30f, 0.06f, 0.80f);
        colors[ImGuiCol_TabActive]              = ImVec4(0.02f, 0.68f, 0.08f, 1.00f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.08f, 0.31f, 0.05f, 0.97f);
        colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.28f, 0.55f, 0.36f, 1.00f);
        colors[ImGuiCol_DockingPreview]         = ImVec4(0.72f, 0.72f, 0.72f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg]         = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.06f, 0.57f, 0.12f, 1.00f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.03f, 0.41f, 0.05f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.12f, 0.69f, 0.26f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.31f, 0.57f, 0.36f, 0.35f);
        colors[ImGuiCol_DragDropTarget]         = ImVec4(0.00f, 1.00f, 0.19f, 0.90f);
        colors[ImGuiCol_NavHighlight]           = ImVec4(0.13f, 0.62f, 0.30f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

        //Apply ImGui configuration options

        //Enable keyboard navigation
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//Enable docking (snapping an ImGui window to another window)
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//Enable multi-viewports (dragging an ImGui window outside the main window)
		imGuiIO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		//Disable saving the ImGui state to a file
		imGuiIO.IniFilename = nullptr;

        //Initialize ImGui
        #ifdef CE_PLATFORM_LINUX
            #ifdef CE_RENDERER_GL
                ImGui_ImplGlfw_InitForOpenGL(nativeWindow, true);
            #endif
        #endif
        #ifdef CE_RENDERER_GL
            bool imGuiSuccess = ImGui_ImplOpenGL3_Init("#version 410");
        #endif

        Asserts::EngineAssert(imGuiSuccess, "ImGui initialization failed!");

        initialized = true;
    }

    void ImGuiWrapper::Shutdown(){
        Asserts::EngineAssert(initialized, "ImGui must be initialized before shutdown!");
        //Shutdown ImGui's backends
		#ifdef CE_PLATFORM_LINUX
            ImGui_ImplGlfw_Shutdown();
        #endif
        #ifdef CE_RENDERER_GL
            ImGui_ImplOpenGL3_Shutdown();
        #endif
		//Destroy the ImGui context
		ImGui::DestroyContext();

        initialized = false;
    }

    void ImGuiWrapper::CreateFrame(){
        Asserts::EngineAssert(initialized, "ImGui must be initialized before new frame!");
        Asserts::EngineAssert(!renderingOK, "Frame already created!");

        //Generate a new ImGui frame
        #ifdef CE_RENDERER_GL
		    ImGui_ImplOpenGL3_NewFrame();
        #endif
		#ifdef CE_PLATFORM_LINUX
		    ImGui_ImplGlfw_NewFrame();
        #endif
		ImGui::NewFrame();

        renderingOK = true;
    }

    void ImGuiWrapper::RenderFrame(){
        Asserts::EngineAssert(initialized, "ImGui must be initialized before drawing frame!");
        Asserts::EngineAssert(renderingOK, "ImGui has not created a new frame!");

        //Get ImGui IO instance
        ImGuiIO& imGuiIO = ImGui::GetIO();

        //Resize ImGui viewport
        glm::i32vec2 windowSize = Window::GetSize();
        imGuiIO.DisplaySize = ImVec2(windowSize.x, windowSize.y);

        //Render ImGui data
		ImGui::Render();

        //Render the ImGui draw data
		#ifdef CE_RENDERER_GL
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        #endif

        //Is multi-viewport enabled?
		if(imGuiIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			//Update and render the ImGui viewports
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			//Apply our current context after viewport rendering
			Window::EnsureWindowRenderContext();
		}

        drawingOK = false;
    }
}