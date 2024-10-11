#pragma once

#include "Utilities/MiscUtils.hpp"

#include "glad/gl.h"

namespace Cacao {
	//Struct for data required for an OpenGL cubemap
	struct Cubemap::CubemapData {
		GLuint gpuID;
	};
}