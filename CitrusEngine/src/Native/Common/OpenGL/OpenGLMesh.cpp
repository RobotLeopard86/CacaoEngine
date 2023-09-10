#include "Native/Common/OpenGL/OpenGLMesh.h"

#include "Core/Log.h"

namespace CitrusEngine {

    Mesh* Mesh::CreateMesh(std::vector<glm::vec3> vertices, std::vector<glm::u32vec3> indices) {
        return new OpenGLMesh(vertices, indices);
    }

    OpenGLMesh::OpenGLMesh(std::vector<glm::vec3> vertices, std::vector<glm::u32vec3> indices) 
        : vertices(vertices), indices(indices), compiled(false), bound(false) {}

    OpenGLMesh::~OpenGLMesh(){
        //Clean up OpenGL objects
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &indexBuffer);
        glDeleteVertexArrays(1, &vertexArray);
    }

    void OpenGLMesh::Compile() {
        //Check if the mesh is compiled already
        if(compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot compile already compiled mesh!");
            return;
        }

        //Unpack mesh data into OpenGL-compatible format

        //Get size of vertex data
        int numVertices = vertices.size();
        float vertexBufferData[numVertices * 3];
        //Populate vertex buffer
        for(int i = 0; i < numVertices; i++){
            glm::vec3 vertex = vertices.at(i);
            vertexBufferData[i * 3] = vertex.x;
            vertexBufferData[(i * 3) + 1] = vertex.y;
            vertexBufferData[(i * 3) + 2] = vertex.z;
        }

        //Get size of index data
        int numIndices = indices.size();
        unsigned int indexBufferData[numIndices * 3];
        //Populate index buffer
        for(int i = 0; i < numIndices; i++){
            glm::u32vec3 index = indices.at(i);
            indexBufferData[i * 3] = index.x;
            indexBufferData[(i * 3) + 1] = index.y;
            indexBufferData[(i * 3) + 2] = index.z;
        }

        //Create OpenGL objects and bind them to a vertex array
        glGenVertexArrays(1, &vertexArray);
        glGenBuffers(1, &vertexBuffer);
        glGenBuffers(1, &indexBuffer);

        glBindVertexArray(vertexArray);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (numVertices * 3), vertexBufferData, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * (numIndices * 3), indexBufferData, GL_STATIC_DRAW);

        //Configure OpenGL vertex buffer interpretation
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //Ensure OpenGL applies state to our vertex array
        glBindVertexArray(vertexArray);

        //Release vertex array from OpenGL once it is saved
        glBindVertexArray(0);

        compiled = true;
    }

    void OpenGLMesh::Release(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot release uncompiled mesh!");
            return;
        }
        if(bound){
            Logging::EngineLog(LogLevel::Error, "Cannot release bound mesh!");
            return;
        }
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &indexBuffer);
        glDeleteVertexArrays(1, &vertexArray);
    }

    void OpenGLMesh::Bind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot bind uncompiled mesh!");
            return;
        }
        if(bound){
            Logging::EngineLog(LogLevel::Error, "Cannot bind already bound mesh!");
            return;
        }
        glBindVertexArray(vertexArray);
        bound = true;
    }

    void OpenGLMesh::Unbind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind uncompiled mesh!");
            return;
        }
        if(!bound){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind unbound mesh!");
            return;
        }
        glBindVertexArray(0);
        bound = false;
    }
}
