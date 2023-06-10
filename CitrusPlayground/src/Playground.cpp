#include "Citrus.h"

#include "glm/gtc/matrix_transform.hpp"

#include <string>
#include <sstream>

using namespace CitrusEngine;

class PlaygroundClient : public CitrusClient {
public:
    PlaygroundClient() { id = "citrus-playground"; windowSize = {1280, 720}; }

    void ClientOnStartup() override {
        vertices.push_back({0.5, 0.5, 0.5});
        vertices.push_back({-0.5, 0.5, 0.5});
        vertices.push_back({0.5, -0.5, 0.5});
        vertices.push_back({-0.5, -0.5, 0.5});
        vertices.push_back({0.5, 0.5, -0.5});
        vertices.push_back({-0.5, 0.5, -0.5});
        vertices.push_back({0.5, -0.5, -0.5});
        vertices.push_back({-0.5, -0.5, -0.5});

        indices.push_back({0, 1, 3});
        indices.push_back({0, 2, 3});
        indices.push_back({4, 5, 7});
        indices.push_back({4, 6, 7});
        indices.push_back({0, 4, 6});
        indices.push_back({0, 2, 6});
        indices.push_back({1, 3, 7});
        indices.push_back({1, 5, 7});
        indices.push_back({0, 1, 5});
        indices.push_back({0, 4, 5});
        indices.push_back({2, 3, 7});
        indices.push_back({2, 6, 7});

        mesh = Mesh::CreateMesh(vertices, indices);
        mesh->Compile();

        transform = new Transform({0, 0, 0}, {0, 0, 0}, {0, 0, 0});

        std::string vertexShaderSource = R"(
            #version 330 core

            layout(location=0) in vec3 pos;
            out vec3 position;

            uniform mat4 transform;
            uniform mat4 camview;

            void main() {
                position = pos;
                gl_Position = camview * transform * vec4(pos, 1.0);
            }
        )";
        
        std::string fragmentShaderSource = R"(
            #version 330 core

            out vec4 color;
            in vec3 position;

            void main() {
                color = vec4(position, 1.0);
            }
        )";
        
        shader = Shader::CreateShader(vertexShaderSource, fragmentShaderSource);
        shader->Compile();

        cam = new PerspectiveCamera(30, 1.7777777777777f);
        cam->SetPosition({-1, 0, 0});

        Renderer::GetInstance()->SetClearColor({25, 25, 25});
        Renderer::GetInstance()->SetCamera(cam);

        uiDrawConsumer = new EventConsumer(BIND_MEMBER_FUNC(PlaygroundClient::OnImGuiDraw));
        GetEventManager()->SubscribeConsumer("ImGuiDraw", uiDrawConsumer);
    }

    void ClientOnShutdown() override {
        delete mesh;
        delete transform;
        delete shader;
        delete cam;
        delete uiDrawConsumer;
    }

    void ClientOnDynamicTick(double timestep) override {
        Renderer::GetInstance()->Clear();

        glm::vec3 camRotChange = glm::vec3(0.0f);
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_J)){
            camRotChange.y -= 0.5f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_K)){
            camRotChange.y += 0.5f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_Y)){
            camRotChange.x -= 0.5f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_U)){
            camRotChange.x += 0.5f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_X)){
            camRotChange.z -= 0.5f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_C)){
            camRotChange.z += 0.5f;
        }
        currentRot = cam->GetRotation();
        glm::vec3 pastLook = cam->GetLookTarget();
        glm::vec3 pastRot = glm::vec3(currentRot);
        currentRot += camRotChange;
        
        if(currentRot.y < 0){
            currentRot.y = 360.0f;
        }
        if(currentRot.y > 360) {
            currentRot.y = 0.0f;
        }

        glm::vec3 posChange = glm::vec3(0.0f);
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_W)){
            posChange.z += 0.01f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_S)){
            posChange.z -= 0.01f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_A)){
            posChange.x -= 0.01f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_D)){
            posChange.x += 0.01f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_E)){
            posChange.y += 0.01f;
        }
        if(Input::GetInstance()->IsKeyPressed(CITRUS_KEY_Q)){
            posChange.y -= 0.01f;
        }
        currentPos = cam->GetPosition() + posChange;

        cam->SetRotation(currentRot);
        cam->SetPosition(currentPos);

        glm::vec3 lookDiff = glm::vec3((cam->GetLookTarget() - pastLook));

        Renderer::GetInstance()->RenderGeometry(mesh, transform, shader);
    }
    void ClientOnFixedTick() override {}

    void OnImGuiDraw(Event& e){
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode);

        std::stringstream sscam;
        sscam << "Camera is at " << std::to_string(currentPos.x) << ", " << std::to_string(currentPos.y) << ", " << std::to_string(currentPos.z) << ".";
        std::stringstream ssvs;
        ssvs << "VSync is currently " << (Window::IsVSyncEnabled() ? "on" : "off");

        ImGui::Begin("Citrus Playground");
        ImGui::Text("This is a demo designed to showcase Citrus Engine.");
        if(ImGui::Button("Toggle VSync")){
            Window::SetVSyncEnabled(!Window::IsVSyncEnabled());
        }
        ImGui::Text("%s", ssvs.str().c_str());
        ImGui::Text("%s", sscam.str().c_str());
        ImGui::End();
    }
private:
    Mesh* mesh;
    Transform* transform;
    Shader* shader;
    PerspectiveCamera* cam;

    std::vector<glm::vec3> vertices;
    std::vector<glm::u32vec3> indices;

    glm::vec3 currentPos, currentRot;

    EventConsumer* uiDrawConsumer;
};

CitrusClient* CreateClient() {
    return new PlaygroundClient();
}