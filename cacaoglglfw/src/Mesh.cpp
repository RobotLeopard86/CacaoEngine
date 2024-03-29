#include "3D/Mesh.hpp"
#include "GLMeshData.hpp"
#include "Core/Log.hpp"
#include "Core/Assert.hpp"
#include "Core/Engine.hpp"
#include "GLUtils.hpp"

#include <future>

#include "glad/gl.h"
#include "glm/gtc/type_ptr.hpp"

//For my sanity
#define nd ((GLMeshData*)nativeData)

namespace Cacao {
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices)
		: vertices(vertices), indices(indices), compiled(false) {
		//Create native data
		nativeData = new GLMeshData();
	}

	std::shared_future<void> Mesh::Compile(){
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()){
			//Invoke OpenGL on the main thread
			return InvokeGL([this]() {
				this->Compile();
			});
		}
		if(compiled){
			Logging::EngineLog("Cannot compile already compiled mesh!", LogLevel::Error);
			return {};
		}

		//Generate buffers and vertex array
		glGenVertexArrays(1, &nd->vao);
		glGenBuffers(1, &nd->vbo);
		glGenBuffers(1, &nd->ibo);

		//Unpack index buffer data from vec3 to floats
		unsigned int ibd[indices.size() * 3];
		for(int i = 0; i < indices.size(); i++){
			glm::vec3 idx = indices[i];
			ibd[i * 3] = idx.x;
			ibd[(i * 3) + 1] = idx.y;
			ibd[(i * 3) + 2] = idx.z;
		}

		//Bind vertex array
		glBindVertexArray(nd->vao);

		//Bind vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, nd->vbo);
		//Load vertex buffer with data
		glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(Vertex)), &vertices[0], GL_STATIC_DRAW);

		//Configure vertex buffer layout
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

		//Bind index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nd->ibo);
		//Load index buffer with data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * 3 * sizeof(unsigned int)), ibd, GL_STATIC_DRAW);

		//Save vertex array state
		glBindVertexArray(nd->vao);
		glBindVertexArray(0);

		compiled = true;

		//Return an empty future
		return {};
	}

	void Mesh::Release(){
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()){
			//Invoke OpenGL on the main thread
			InvokeGL([this]() {
				this->Release();
			});
			return;
		}
		if(!compiled){
			Logging::EngineLog("Cannot release uncompiled mesh!", LogLevel::Error);
			return;
		}

		//Delete buffers
		glDeleteBuffers(1, &nd->vbo);
		glDeleteBuffers(1, &nd->ibo);

		compiled = false;
	}

	void Mesh::Draw(){
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()){
			Logging::EngineLog("Cannot draw mesh in non-rendering thread!", LogLevel::Error);
			return;
		}
		if(!compiled){
			Logging::EngineLog("Cannot draw uncompiled mesh!", LogLevel::Error);
			return;
		}

		//Bind vertex array
		glBindVertexArray(nd->vao);

		//Draw object
		glDrawElements(GL_TRIANGLES, (indices.size() * 3), GL_UNSIGNED_INT, nullptr);

		//Unbind vertex array
		glBindVertexArray(0);
	}
}