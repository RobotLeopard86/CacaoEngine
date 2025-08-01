#include "OpenGLMesh.hpp"
#include "Cacao/Engine.hpp"
#include "Module.hpp"

namespace Cacao {
	std::optional<std::shared_future<void>> OpenGLMeshImpl::Realize(bool& success) {
		//Unpack index buffer data
		std::vector<unsigned int> ibd(indices.size() * 3);
		for(unsigned int i = 0; i < indices.size(); ++i) {
			glm::vec3 idx = indices[i];
			ibd[i * 3] = idx.x;
			ibd[(i * 3) + 1] = idx.y;
			ibd[(i * 3) + 2] = idx.z;
		}

		//Open-GL specific stuff needs to be on the main thread
		return Engine::Get().RunTaskOnMainThread([this, &ibd, &success]() {
			//Generate buffers and vertex array
			glGenVertexArrays(1, &vao);
			glGenBuffers(1, &vbo);
			glGenBuffers(1, &ibo);

			//Bind vertex array
			glBindVertexArray(vao);

			//Bind vertex buffer
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
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
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			//Load index buffer with data
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * 3 * sizeof(unsigned int)), &ibd[0], GL_STATIC_DRAW);

			//Save vertex array state
			glBindVertexArray(vao);
			glBindVertexArray(0);

			success = true;
		});
	}

	void OpenGLMeshImpl::DropRealized() {
		Engine::Get().RunTaskOnMainThread([this]() {
			//Delete buffers and vertex array
			glDeleteBuffers(1, &vbo);
			glDeleteBuffers(1, &ibo);
			glDeleteVertexArrays(1, &vao);

			//Zero buffer names to avoid confusion
			vbo = 0;
			ibo = 0;
			vao = 0;
		});
	}

	Mesh::Impl* OpenGLModule::ConfigureMesh() {
		return new OpenGLMeshImpl();
	}
}