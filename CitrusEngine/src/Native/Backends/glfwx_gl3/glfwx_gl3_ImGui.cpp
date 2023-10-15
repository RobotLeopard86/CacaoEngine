#include "ImGui/ImGuiWrapper.hpp"

#include "Core/CitrusClient.hpp"

#include "GLFW/glfw3.h"

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

//GLFW+X11+OpenGL3 ImGui Backend

namespace CitrusEngine {

    bool ImGuiWrapper::InitImGuiBackend(){
        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)CitrusClient::GetWindow()->GetNativeWindow(), true);
        return ImGui_ImplOpenGL3_Init("#version 330");
    }

    void ImGuiWrapper::ShutdownImGuiBackend(){
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }

    void ImGuiWrapper::ImGuiBackendNewFrame(){
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
    }

    void ImGuiWrapper::ImGuiBackendRender(ImDrawData* drawData){
        ImGui_ImplOpenGL3_RenderDrawData(drawData);
    }

    void ImGuiWrapper::ImGuiBackendPostViewport(){
        glfwMakeContextCurrent((GLFWwindow*)CitrusClient::GetWindow()->GetNativeWindow());
    }
}