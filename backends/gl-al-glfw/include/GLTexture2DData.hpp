#pragma once

#include "Utilities/MiscUtils.hpp"

#include "glad/gl.h"

namespace Cacao {
	//Struct for data required for an OpenGL 2D texture
	struct GLTexture2DData : public NativeData {
		GLuint gpuID;
		GLenum format;
	};
}