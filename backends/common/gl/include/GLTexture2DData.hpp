#pragma once

#include "Graphics/Textures/Texture2D.hpp"

#include "glad/gl.h"

namespace Cacao {
	//Struct for data required for an OpenGL 2D texture
	struct Texture2D::Tex2DData {
		GLuint gpuID;
		GLenum format;
	};
}