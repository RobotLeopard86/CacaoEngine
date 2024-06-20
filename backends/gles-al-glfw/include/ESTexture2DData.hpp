#pragma once

#include "Utilities/MiscUtils.hpp"

#include "glad/gles2.h"

namespace Cacao {
	//Struct for data required for an OpenGL ES 2D texture
	struct ESTexture2DData : public NativeData {
		GLuint gpuID;
		GLenum format;
	};
}