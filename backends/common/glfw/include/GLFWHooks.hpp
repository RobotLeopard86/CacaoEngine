#pragma once

#include "GLFW/glfw3.h"

//Defines a set of "hook" functions that are called during the common SDL code to execute graphics API-specific code

namespace Cacao {
	void SetGLFWHints();
	void SetupGraphicsAPI(GLFWwindow* win);
	void CleanupGraphicsAPI();
	void ResizeViewport(GLFWwindow* win);
}