#pragma once

#include "glad/gl.h"

#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	//Struct for data required for an OpenGL mesh
	struct Mesh::MeshData {
		GLuint vao, vbo, ibo;
	};
}