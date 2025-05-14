#pragma once

#include "Targetgen.hpp"

#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"

using Slang::ComPtr;

namespace Targetgen {
	struct CompiledShaderObject {
		ComPtr<slang::IGlobalSession> gsession;
		ComPtr<slang::ISession> session;
		ComPtr<slang::IComponentType> linked;
	};

	CompiledShaderObject SetupCSO(ibytestream& in, SlangCompileTarget tgt, const std::string& profile);
}