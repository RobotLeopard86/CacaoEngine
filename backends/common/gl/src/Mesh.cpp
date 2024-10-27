#include "3D/Mesh.hpp"
#include "GLMeshData.hpp"
#include "Core/Log.hpp"
#include "Core/Exception.hpp"
#include "Core/Engine.hpp"
#include "GLUtils.hpp"

#include <future>

#include "glad/gl.h"
#include "glm/gtc/type_ptr.hpp"

namespace Cacao {
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices)
	  : Asset(false), vertices(vertices), indices(indices) {
		//Create native data
		nativeData.reset(new MeshData());
	}

	std::shared_future<void> Mesh::Compile() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Invoke OpenGL on the main thread
			return InvokeGL([this]() {
				this->Compile();
			});
		}
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled mesh!");

		//Generate buffers and vertex array
		glGenVertexArrays(1, &nativeData->vao);
		glGenBuffers(1, &nativeData->vbo);
		glGenBuffers(1, &nativeData->ibo);

		//Unpack index buffer data from vec3 to floats
		std::vector<unsigned int> ibd(indices.size() * 3);
		for(int i = 0; i < indices.size(); i++) {
			glm::vec3 idx = indices[i];
			ibd[i * 3] = idx.x;
			ibd[(i * 3) + 1] = idx.y;
			ibd[(i * 3) + 2] = idx.z;
		}

		//Bind vertex array
		glBindVertexArray(nativeData->vao);

		//Bind vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, nativeData->vbo);
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
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		//Bind index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nativeData->ibo);
		//Load index buffer with data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * 3 * sizeof(unsigned int)), &ibd[0], GL_STATIC_DRAW);

		//Save vertex array state
		glBindVertexArray(nativeData->vao);
		glBindVertexArray(0);

		compiled = true;

		//Return an empty future
		return {};
	}

	void Mesh::Release() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Try to invoke OpenGL and throw any exceptions back to the initial caller
			try {
				InvokeGL([this]() {
					this->Release();
				}).get();
				return;
			} catch(...) {
				std::rethrow_exception(std::current_exception());
			}
		}
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled mesh!");

		//Delete buffers
		glDeleteBuffers(1, &nativeData->vbo);
		glDeleteBuffers(1, &nativeData->ibo);

		compiled = false;
	}

	void Mesh::Draw() {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot draw mesh in non-rendering thread!");
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot draw uncompiled mesh!");

		//Bind vertex array
		glBindVertexArray(nativeData->vao);

		//Draw object
		glDrawElements(GL_TRIANGLES, (indices.size() * 3), GL_UNSIGNED_INT, nullptr);

		//Unbind vertex array
		glBindVertexArray(0);
	}
}