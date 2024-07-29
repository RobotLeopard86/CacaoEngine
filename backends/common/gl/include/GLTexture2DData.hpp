#pragma once

#include "Utilities/MiscUtils.hpp"

#include "GLHeaders.hpp"

namespace Cacao {
	//Struct for data required for an OpenGL (ES) 2D texture
	struct Texture2D::Tex2DData {
		GLuint gpuID;
		GLenum format;
	};
}