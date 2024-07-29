#pragma once

#include "GLFW/glfw3.h"

#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	//Struct for data required for a GLFW window
	struct Window::WindowData {
		GLFWwindow* win;
	};
}