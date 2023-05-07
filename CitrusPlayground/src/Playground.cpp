#include "Citrus.h"

class PlaygroundClient : public CitrusEngine::CitrusClient {
public:
    PlaygroundClient() { id = "citrus-playground"; windowSize = {1280, 720}; }

    void ClientOnStartup() override {
        std::vector<glm::vec3> vertices;
        vertices.push_back({0.5, 0.5, 0.5});
        vertices.push_back({-0.5, 0.5, 0.5});
        vertices.push_back({0.5, -0.5, 0.5});
        vertices.push_back({-0.5, -0.5, 0.5});
        vertices.push_back({0.5, 0.5, -0.5});
        vertices.push_back({-0.5, 0.5, -0.5});
        vertices.push_back({0.5, -0.5, -0.5});
        vertices.push_back({-0.5, -0.5, -0.5});

        std::vector<glm::u32vec3> indices;
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

            uniform mat4 transform;
            uniform mat4 camview;

            void main() {
                gl_Position = camview * transform * vec4(pos, 1.0);
            }
        )";

        std::string fragmentShaderSource = R"(
            #version 330 core

            layout(location=0) out vec4 color;

            void main() {
                color = vec4(0.0, 0.45, 1.0, 1.0);
            }
        )";

        shader = CitrusEngine::Shader::CreateShader(vertexShaderSource, fragmentShaderSource);
        shader->Compile();

        cam = new CitrusEngine::PerspectiveCamera(60, windowSize);
        cam->SetPosition({-1, 0, 0});

        CitrusEngine::Renderer::SetClearColor({25, 25, 25});
        CitrusEngine::Renderer::SetCamera(cam);
    }

    void ClientOnShutdown() override {
        delete mesh;
        delete transform;
        delete shader;
        delete cam;
    }

    void ClientOnDynamicTick(double timestep) override {
        CitrusEngine::Renderer::Clear();

        float camRotChange = 0.0f;
        if(CitrusEngine::Input::IsKeyPressed(CITRUS_KEY_A)){
            camRotChange -= 0.5f;
        }
        if(CitrusEngine::Input::IsKeyPressed(CITRUS_KEY_D)){
            camRotChange += 0.5f;
        }
        glm::vec3 currentRot = cam->GetRotation();
        currentRot.y += camRotChange;
        
        if(currentRot.y < 0){
            currentRot.y = 360.0f;
        }
        if(currentRot.y > 360) {
            currentRot.y = 0.0f;
        }

        cam->SetRotation(currentRot);

        if(CitrusEngine::Input::IsMouseButtonPressed(CITRUS_MOUSE_BUTTON_RIGHT)){
            CitrusEngine::Logging::ClientLog(CitrusEngine::LogLevel::Info, "Current camera rotation is: {" + std::to_string(currentRot.x) + ", " + std::to_string(currentRot.y) + ", " + std::to_string(currentRot.z) + "}.");
        }

        CitrusEngine::Renderer::RenderGeometry(mesh, transform, shader);
    }
    void ClientOnFixedTick() override {}
private:
    CitrusEngine::Mesh* mesh;
    CitrusEngine::Transform* transform;
    CitrusEngine::Shader* shader;
    CitrusEngine::PerspectiveCamera* cam;
};

CitrusEngine::CitrusClient* CreateClient() {
    return new PlaygroundClient();
}