#pragma once

#include "GLHeaders.hpp"

#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	//Struct for data required for an OpenGL (ES) mesh
	struct Mesh::MeshData {
		GLuint vao, vbo, ibo;
	};
}