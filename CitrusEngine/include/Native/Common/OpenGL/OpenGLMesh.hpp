#pragma once

#include "Graphics/Mesh.hpp"

#include "glad/gl.h"

namespace CacaoEngine {

    //OpenGL implementation of Mesh (see Mesh.hpp for method details)
    class OpenGLMesh : public Mesh {
    public:
        OpenGLMesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices);
        ~OpenGLMesh();

        void Compile() override;
        void Release() override;
        void Draw(Shader* shader, Transform* transform) override;
        void PureDraw() override;
    private:
        GLuint vertexArray, vertexBuffer, indexBuffer;

        std::vector<Vertex> vertices;
        std::vector<glm::uvec3> indices;
    };
}