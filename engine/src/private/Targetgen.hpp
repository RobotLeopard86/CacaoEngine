#pragma once

#include "Bytestream.hpp"
#include "DllHelper.hpp"

#include <vector>
#include <cstdint>

#include "slang.h"
#include "slang-com-ptr.h"

namespace Cacao {

	struct CompiledShaderObject {
		Slang::ComPtr<slang::IGlobalSession> gsession;
		Slang::ComPtr<slang::ISession> session;
		Slang::ComPtr<slang::IComponentType> linked;
	};

	CompiledShaderObject SetupCSO(ibytestream& in, SlangCompileTarget tgt, const std::string& profile);

#ifdef HAS_GL
	struct CACAO_API GLSL {
		std::string vertex, fragment;
	};

	CACAO_API GLSL GenerateGLSL(ibytestream& in, bool es);
#endif

	CACAO_API std::vector<uint32_t> GenerateSPV(ibytestream& in);

}