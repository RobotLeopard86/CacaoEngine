#pragma once

#include "Utilities/MiscUtils.hpp"

#include "glad/gles2.h"

namespace Cacao {
	//Struct for data required for an OpenGL ES cubemap
	struct ESCubemapData : public NativeData {
		GLuint gpuID;
	};
}