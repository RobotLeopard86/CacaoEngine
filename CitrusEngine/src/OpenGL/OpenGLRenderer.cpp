#include "OpenGLRenderer.h"

#include "glad/glad.h"

namespace CitrusEngine {

    OpenGLRenderer::OpenGLRenderer() {
        clearColor = glm::vec4(1.0);
    }

    void OpenGLRenderer::SetClearColor_Impl(glm::i8vec3 color) {
        clearColor = glm::vec4((color.r / 256), (color.g / 256), (color.b / 256), 1.0);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    }

    void OpenGLRenderer::Clear_Impl(){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRenderer::RenderGeometry_Impl(Mesh mesh, Transform transform, Shader shader) {
        //Unpack mesh data into OpenGL-compatible format

        //Get mesh data
        glm::vec3* vertices = mesh.vertices.data();
        glm::i32vec3* indices = mesh.indices.data();

        //Get sizes of data
        int vertexSize = sizeof(vertices);
        int numVertices = vertexSize / sizeof(glm::vec3);
        float* vertexBufferData = new float[numVertices * 3];
        //Populate buffer data
        for(int i = 0; i < numVertices; i++){
            vertexBufferData[i] = vertices[i].x;
            vertexBufferData[i + 1] = vertices[i].y;
            vertexBufferData[i + 2] = vertices[i].z;
        }
        //Get sizes of data
        int indexSize = sizeof(indices);
        int numIndices = indexSize / sizeof(glm::vec3);
        int* indexBufferData = new int[numIndices * 3];
        //Populate buffer data
        for(int i = 0; i < numIndices; i++){
            indexBufferData[i] = indices[i].x;
            indexBufferData[i + 1] = indices[i].y;
            indexBufferData[i + 2] = indices[i].z;
        }

        //Create OpenGL buffers
        uint32_t vertexArray;
        glCreateVertexArrays(1, &vertexArray);

        uint32_t vertexBuffer;
        glCreateBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertexSize, vertexBufferData, GL_STATIC_DRAW);

        uint32_t indexBuffer;
        glCreateBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices, indexBufferData, GL_STATIC_DRAW);

        //Bind buffers to OpenGL for drawing
        glBindVertexArray(vertexArray);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        //Bind shader
        shader.Bind();

        //Configure OpenGL rendering
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        //Draw geometry
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

        //Clean up
        delete[] vertices;
        delete[] indices;
    }

    Renderer* Renderer::CreateNativeRenderer(){
        return new OpenGLRenderer();
    }
}