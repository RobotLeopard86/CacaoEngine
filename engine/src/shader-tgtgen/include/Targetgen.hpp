#pragma once

#include "Bytestream.hpp"
#include "DllHelper.hpp"

#include <vector>
#include <cstdint>

namespace Targetgen {

#ifdef BE_OPENGL
	struct CACAO_API GLSL {
		std::string vertex, fragment;
	};

	CACAO_API GLSL GenerateGLSL(ibytestream& in, bool es);
#endif

	CACAO_API std::vector<uint32_t> GenerateSPV(ibytestream& in);

}