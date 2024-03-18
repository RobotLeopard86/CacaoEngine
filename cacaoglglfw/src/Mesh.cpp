#include "3D/Mesh.hpp"
#include "GLMeshData.hpp"
#include "Core/Log.hpp"
#include "Core/Assert.hpp"

#include "glad/gl.h"

//For my sanity
#define nd ((GLMeshData*)nativeData)

namespace Cacao {
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices)
		: vertices(vertices), indices(indices) {
		//Create native data
		nativeData = new GLMeshData();
	}

	void Mesh::Compile(){
		if(compiled){
			Logging::EngineLog("Cannot compile already compiled mesh!", LogLevel::Error);
			return;
		}

		//Generate vertex arrays
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

		//Bind vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, nd->vbo);
		//Load vertex buffer with data
		glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(Vertex)), &vertices[0], GL_STATIC_DRAW);

		//Bind index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nd->ibo);
		//Load index buffer with data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * 3 * sizeof(unsigned int)), ibd, GL_STATIC_DRAW);

		//Unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		compiled = true;
	}

	void Mesh::Release(){
		//Delete vertex array assets
		glDeleteBuffers(1, &nd->vbo);
		glDeleteBuffers(1, &nd->ibo);

		compiled = false;
	}

	void Mesh::Draw(){
		//Create and bind temporary vertex array
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		//Bind buffers
		glBindBuffer(GL_ARRAY_BUFFER, nd->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nd->ibo);

		//Configure vertex buffer layout
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

		//Draw object
		glDrawElements(GL_TRIANGLES, (indices.size() * 3), GL_UNSIGNED_INT, nullptr);

		//Clean up temporary VAO
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &vao);
	}
}