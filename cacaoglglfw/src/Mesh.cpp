#include "3D/Mesh.hpp"
#include "GLMeshData.hpp"
#include "Core/Log.hpp"
#include "Core/Assert.hpp"
#include "Core/Engine.hpp"
#include "Events/EventSystem.hpp"

#include "glad/gl.h"

//For my sanity
#define nd ((GLMeshData*)nativeData)

namespace Cacao {
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices)
		: vertices(vertices), indices(indices), compiled(false) {
		//Create native data
		nativeData = new GLMeshData();
	}

	void Mesh::Compile(){
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			DataEvent<Mesh*> de("MeshCompile", this);
			EventManager::GetInstance()->Dispatch(de);
			return;
		} 

		if(compiled){
			Logging::EngineLog("Cannot compile already compiled mesh!", LogLevel::Error);
			return;
		}

		//Generate vertex arrays and buffers
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

		//Bind VAO
		glBindVertexArray(nd->vao);

		//Bind vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, nd->vbo);
		//Load vertex buffer with data
		glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(Vertex)), &vertices[0], GL_STATIC_DRAW);

		//Bind index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nd->ibo);
		//Load index buffer with data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * 3 * sizeof(unsigned int)), ibd, GL_STATIC_DRAW);

		//Save VAO and unbind
		glBindVertexArray(nd->vao);
		glBindVertexArray(0);

		compiled = true;
	}

	void Mesh::Release(){
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			DataEvent<Mesh*> de("MeshRelease", this);
			EventManager::GetInstance()->Dispatch(de);
			return;
		} 

		if(!compiled){
			Logging::EngineLog("Cannot release uncompiled mesh!", LogLevel::Error);
			return;
		}

		//Delete vertex array assets
		glDeleteBuffers(1, &nd->vbo);
		glDeleteBuffers(1, &nd->ibo);
		glDeleteVertexArrays(1, &nd->vao);

		compiled = false;
	}

	void Mesh::Draw(){
		if(!compiled){
			Logging::EngineLog("Cannot draw uncompiled mesh!", LogLevel::Error);
			return;
		}

		//Draw object
		glBindVertexArray(nd->vao);
		glDrawElements(GL_TRIANGLES, (indices.size() * 3), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}
}