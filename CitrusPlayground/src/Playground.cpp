#include "Citrus.h"

#include "glm/gtc/matrix_transform.hpp"

class PlaygroundClient : public CitrusEngine::CitrusClient {
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

        mesh = CitrusEngine::Mesh::CreateMesh(vertices, indices);
        mesh->Compile();

        transform = new CitrusEngine::Transform({0, 0, 0}, {0, 0, 0}, {0, 0, 0});

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
        
        shader = CitrusEngine::Shader::CreateShader(vertexShaderSource, fragmentShaderSource);
        shader->Compile();

        cam = new CitrusEngine::PerspectiveCamera(60, ((float)windowSize.x / (float)windowSize.y));
        cam->SetPosition({-1, 0, 0});

        CitrusEngine::Renderer::GetInstance()->SetClearColor({25, 25, 25});
        CitrusEngine::Renderer::GetInstance()->SetCamera(cam);
    }

    void ClientOnShutdown() override {
        delete mesh;
        delete transform;
        delete shader;
        delete cam;
    }

    void ClientOnDynamicTick(double timestep) override {
        CitrusEngine::Renderer::GetInstance()->Clear();

        float camRotChange = 0.0f;
        if(CitrusEngine::Input::GetInstance()->IsKeyPressed(CITRUS_KEY_LEFT)){
            camRotChange -= 0.1f;
        }
        if(CitrusEngine::Input::GetInstance()->IsKeyPressed(CITRUS_KEY_RIGHT)){
            camRotChange += 0.1f;
        }
        glm::vec3 currentRot = cam->GetRotation();
        currentRot.y += camRotChange;
        
        if(currentRot.y < 0){
            currentRot.y = 360.0f;
        }
        if(currentRot.y > 360) {
            currentRot.y = 0.0f;
        }

        glm::vec3 posChange = glm::vec3(0.0f);
        if(CitrusEngine::Input::GetInstance()->IsKeyPressed(CITRUS_KEY_W)){
            posChange.x += 0.0001f;
        }
        if(CitrusEngine::Input::GetInstance()->IsKeyPressed(CITRUS_KEY_S)){
            posChange.x -= 0.0001f;
        }
        if(CitrusEngine::Input::GetInstance()->IsKeyPressed(CITRUS_KEY_A)){
            posChange.z -= 0.0001f;
        }
        if(CitrusEngine::Input::GetInstance()->IsKeyPressed(CITRUS_KEY_D)){
            posChange.z += 0.0001f;
        }
        if(CitrusEngine::Input::GetInstance()->IsKeyPressed(CITRUS_KEY_E)){
            posChange.y -= 0.0001f;
        }
        if(CitrusEngine::Input::GetInstance()->IsKeyPressed(CITRUS_KEY_Q)){
            posChange.y += 0.0001f;
        }
        glm::vec3 currentPos = cam->GetPosition();
        currentPos += posChange;

        cam->SetRotation(currentRot);
        cam->SetPosition(currentPos);

        if(CitrusEngine::Input::GetInstance()->IsMouseButtonPressed(CITRUS_MOUSE_BUTTON_RIGHT)){
            glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0), transform->pos);
            glm::mat4 cameraViewMatrix = cam->GetViewProjectionMatrix() * transformMatrix;
            for(int i = 0; i < vertices.size(); i++){
                glm::vec3 adjustedVertex = cameraViewMatrix * glm::vec4(vertices.at(i), 1.0);
                CitrusEngine::Logging::ClientLog(CitrusEngine::LogLevel::Info, "Vertex #" + std::to_string(i + 1) + " display: {" + std::to_string(adjustedVertex.x) + ", " + std::to_string(adjustedVertex.y) + ", " + std::to_string(adjustedVertex.z) + "}.");
            }
        }

        CitrusEngine::Renderer::GetInstance()->RenderGeometry(mesh, transform, shader);
    }
    void ClientOnFixedTick() override {}
private:
    CitrusEngine::Mesh* mesh;
    CitrusEngine::Transform* transform;
    CitrusEngine::Shader* shader;
    CitrusEngine::PerspectiveCamera* cam;

    std::vector<glm::vec3> vertices;
    std::vector<glm::u32vec3> indices;
};

CitrusEngine::CitrusClient* CreateClient() {
    return new PlaygroundClient();
}