#pragma once

#include "impl/Mesh.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLMeshImpl : public Mesh::Impl {
	  public:
		void Bake(bool& success) override;
		void Discard() override;

		//Vertex Array Object, Vertex Buffer Object, Index (Element) Buffer Object
		GLuint vao, vbo, ibo;
	};
}