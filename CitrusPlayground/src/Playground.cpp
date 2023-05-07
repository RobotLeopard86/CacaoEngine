#include "Citrus.h"

class PlaygroundClient : public CitrusEngine::CitrusClient {
public:
    PlaygroundClient() { id = "citrus-playground"; }

    void ClientOnStartup() override {
        std::vector<glm::vec3> vertices;
        vertices.push_back({0.5, 0.5, 0.0});
        vertices.push_back({0.5, -0.5, 0.0});
        vertices.push_back({-0.5, 0.5, 0.0});
        vertices.push_back({-0.5, -0.5, 0.0});

        std::vector<glm::u32vec3> indices;
        indices.push_back({0, 1, 2});
        indices.push_back({2, 3, 1});

        mesh = CitrusEngine::Mesh::CreateMesh(vertices, indices);
        mesh->Compile();
        
        transform = new CitrusEngine::Transform({0, 0, 0}, {0, 0, 0}, {0, 0, 0});

        std::string vertexShaderSource = R"(
            #version 330 core

            layout(location=0) in vec3 pos;

            void main() {
                gl_Position = vec4(pos, 1.0);
            }
        )";

        std::string fragmentShaderSource = R"(
            #version 330 core

            layout(location=0) out vec4 color;

            void main() {
                color = vec4(0.5, 1.0, 0.5, 1.0);
            }
        )";

        shader = CitrusEngine::Shader::CreateShader(vertexShaderSource, fragmentShaderSource);
        shader->Compile();

        CitrusEngine::Renderer::SetClearColor({0, 179, 0});
    }

    void ClientOnShutdown() override {
        delete mesh;
        delete transform;
        delete shader;
    }

    void ClientOnDynamicTick(double timestep) override {
        CitrusEngine::Renderer::Clear();
        CitrusEngine::Renderer::RenderGeometry(mesh, transform, shader);
    }
    void ClientOnFixedTick() override {}
private:
    CitrusEngine::Mesh* mesh;
    CitrusEngine::Transform* transform;
    CitrusEngine::Shader* shader;
};

CitrusEngine::CitrusClient* CreateClient() {
    return new PlaygroundClient();
}