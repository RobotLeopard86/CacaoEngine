#pragma once

#if __has_include("renderdoc_app.h")
#define RDOC_ENABLE 1
#include "renderdoc_app.h"

#ifdef _WIN32
#include "../win32/Win32Types.hpp"
#else
#include <dlfcn.h>
#endif

namespace Cacao::RenderDoc {
	inline RENDERDOC_API_1_1_2* rdoc = nullptr;
#ifdef _WIN32
	inline HMODULE rdocLib = NULL;
#else
	inline void* rdocLib = nullptr;
#endif
}
#else
#define RDOC_ENABLE_0
#endif