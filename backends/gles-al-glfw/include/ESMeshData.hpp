#pragma once

#include "glad/gles2.h"

#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	//Struct for data required for an OpenGL ES mesh
	struct ESMeshData : public NativeData {
		GLuint vao, vbo, ibo;
	};
}