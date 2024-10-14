#include "VkHooks.hpp"

#include "Core/Assert.hpp"

#include "GLFW/glfw3.h"

#include <cstring>

namespace Cacao {
	std::vector<const char*> GetRequiredInstanceExts() {
		uint32_t excount = 0;
		const char** exts = glfwGetRequiredInstanceExtensions(&excount);
		int code = glfwGetError(nullptr);
		EngineAssert(exts, "Could not load instance extension list!");
		std::vector<const char*> out;
		out.resize(excount);
		std::memcpy(out.data(), exts, excount * sizeof(const char*));
		return out;
	}
}