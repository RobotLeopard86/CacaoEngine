#pragma once

#include "impl/Mesh.hpp"

#include "glad/gl.h"

namespace Cacao {
	class OpenGLMeshImpl : public Mesh::Impl {
	  public:
		void Realize(bool& success) override;
		void DropRealized() override;

		//Vertex Array Object, Vertex Buffer Object, Index (Element) Buffer Object
		GLuint vao, vbo, ibo;
	};
}