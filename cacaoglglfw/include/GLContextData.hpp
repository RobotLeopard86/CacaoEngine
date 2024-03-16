#pragma once

#include "Utilities/MiscUtils.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

namespace Cacao {
	struct GLContextData : public NativeData {
		GLFWwindow* window;
	};
}