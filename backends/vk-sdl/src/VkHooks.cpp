#include "VkHooks.hpp"

#include "Core/Assert.hpp"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

#include <cstring>

namespace Cacao {
	std::vector<const char*> GetRequiredInstanceExts() {
		uint32_t excount;
		const char* const* exts = SDL_Vulkan_GetInstanceExtensions(&excount);
		EngineAssert(exts, "Could not load instance extension list!");
		std::vector<const char*> out;
		out.resize(excount);
		std::memcpy(out.data(), exts, excount * sizeof(const char*));
		return out;
	}
}