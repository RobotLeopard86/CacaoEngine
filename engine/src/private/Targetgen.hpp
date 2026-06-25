#pragma once

#include "DllHelper.hpp"
#include "libcacaoasset.hpp"

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

	CompiledShaderObject SetupCSO(const libcacaoasset::Shader& in, SlangCompileTarget tgt, const std::string& profile);
	std::pair<std::string, std::string> GetEntrypointNames(const libcacaoasset::Shader& in);

#ifdef HAS_GL
	struct CACAO_API GLSL {
		std::string vertex, fragment;
	};

	CACAO_API GLSL GenerateGLSL(const libcacaoasset::Shader& in, bool es);
#endif

	CACAO_API std::vector<uint32_t> GenerateSPV(const libcacaoasset::Shader& in);

}