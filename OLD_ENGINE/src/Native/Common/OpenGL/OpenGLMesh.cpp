#include "Native/Common/OpenGL/OpenGLMesh.hpp"

#include "Core/Assert.hpp"

#include "Utilities/StateManager.hpp"

namespace CacaoEngine {

    Mesh* Mesh::Create(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices){
        return new OpenGLMesh(vertices, indices);
    }

    OpenGLMesh::OpenGLMesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices)
        : vertices(vertices), indices(indices) {}

    OpenGLMesh::~OpenGLMesh() {
        if(compiled){
            //Release OpenGL assets before destruction
            Release();
        }
    }

    void OpenGLMesh::Compile(){
        if(compiled){
			Logging::EngineLog(LogLevel::Error, "Cannot compile already compiled mesh!");
			return;
		}

        //Generate vertex arrays
        glGenVertexArrays(1, &vertexArray);
        glGenBuffers(1, &vertexBuffer);
        glGenBuffers(1, &indexBuffer);

        //Unpack index buffer data from vec3 to floats
        unsigned int ibd[indices.size() * 3];
        for(int i = 0; i < indices.size(); i++){
            glm::vec3 idx = indices[i];
            ibd[i * 3] = idx.x;
            ibd[(i * 3) + 1] = idx.y;
            ibd[(i * 3) + 2] = idx.z;
        }

        //Bind vertex array to save buffer setup
        glBindVertexArray(vertexArray);

        //Bind vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        //Load vertex buffer with data
        glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(Vertex)), &vertices[0], GL_STATIC_DRAW);

        //Bind index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        //Load index buffer with data
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * 3 * sizeof(unsigned int)), ibd, GL_STATIC_DRAW);

        //Configure OpenGL vertex buffer layout
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

        //Ensure OpenGL saves vertex array state
        glBindVertexArray(vertexArray);

        //Unbind vertex array once saved
        glBindVertexArray(0);

        compiled = true;
    }

    void OpenGLMesh::Release(){
        //Delete vertex array assets
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &indexBuffer);
        glDeleteVertexArrays(1, &vertexArray);

        compiled = false;
    }

    void OpenGLMesh::Draw(Shader* shader, Transform* transform){
        //Bind shader
		shader->Bind();

		//Upload uniforms
        shader->UploadUniformMat4("model", transform->GetTransformationMatrix());
        shader->UploadUniformMat4("view", StateManager::GetInstance()->GetActiveCamera()->GetViewMatrix());
		shader->UploadUniformMat4("projection", StateManager::GetInstance()->GetActiveCamera()->GetProjectionMatrix());

		//Enable face depth sorting
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

		//Draw object
        glBindVertexArray(vertexArray);
        glDrawElements(GL_TRIANGLES, (indices.size() * 3), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
		
		//Unbind shader
        shader->Unbind();
    }

    void OpenGLMesh::PureDraw(){
        glBindVertexArray(vertexArray);
        glDrawElements(GL_TRIANGLES, (indices.size() * 3), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}