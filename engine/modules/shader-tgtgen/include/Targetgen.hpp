#pragma once

#include "libcacaocommon.hpp"

#include <memory>
#include <vector>

namespace Targetgen {

#ifdef BE_OPENGL
	struct GLSL {
		std::string vertex, fragment;
	};

	GLSL GenerateGLSL(ibytestream& in);
#endif

	std::vector<uint32_t> GenerateSPV(ibytestream& in);

}