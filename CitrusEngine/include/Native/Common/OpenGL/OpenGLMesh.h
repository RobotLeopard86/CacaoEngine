#pragma once

#include "Graphics/Mesh.h"

#include "glad/gl.h"

namespace CitrusEngine {

    //OpenGL implementation of Mesh (see Mesh.h for method details)
    class OpenGLMesh : public Mesh {
    public:
        OpenGLMesh(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> indices);
        ~OpenGLMesh() override;

        void Compile() override;
        void Release() override;
        void Bind() override;
        void Unbind() override;
        std::vector<glm::vec3> GetVertices() override { return vertices; }
        std::vector<glm::uvec3> GetIndices() override { return indices; }
    private:
        std::vector<glm::vec3> vertices;
        std::vector<glm::uvec3> indices;
        
        GLuint vertexArray, vertexBuffer, indexBuffer;

        bool compiled;
        bool bound;
    };
}