#pragma once

#include "Utilities/MiscUtils.hpp"

#include "GLHeaders.hpp"

namespace Cacao {
	//Struct for data required for an OpenGL (ES) cubemap
	struct Cubemap::CubemapData {
		GLuint gpuID;
	};
}