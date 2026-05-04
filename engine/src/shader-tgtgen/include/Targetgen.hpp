#pragma once

#include "libcacaocommon.hpp"

#include <vector>
#include <cstdint>

namespace Targetgen {

#ifdef BE_OPENGL
	struct GLSL {
		std::string vertex, fragment;
	};

	GLSL GenerateGLSL(ibytestream& in, bool es);
#endif

	std::vector<uint32_t> GenerateSPV(ibytestream& in);

}