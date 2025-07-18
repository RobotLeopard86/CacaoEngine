#pragma once

#include "impl/Mesh.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLMeshImpl : public Mesh::Impl {
	  public:
		std::optional<std::shared_future<void>> Realize() override;
		void DropRealized() override;
		bool DoWaitAsyncForSync() const override {
			return true;
		}

		//Vertex Array Object, Vertex Buffer Object, Index (Element) Buffer Object
		GLuint vao, vbo, ibo;
	};
}