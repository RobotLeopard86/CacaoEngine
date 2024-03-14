#pragma once

#include "Utilities/MiscUtils.hpp"

#include "glad/gl.h"

namespace Cacao {
	//Struct for data required for an OpenGL skybox
	struct GLSkyboxData : public NativeData {
		GLuint vao, vbo;
	};
}