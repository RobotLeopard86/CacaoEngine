#pragma once

#include "Graphics/Shader.hpp"

#include "GLHeaders.hpp"

//Defines a set of "hook" functions that are called during the common OpenGL/OpenGL ES code to execute variant-specific code

namespace Cacao {
	struct UniformTypeCheckResponse {
		bool ok;
		std::string err;
	};

	UniformTypeCheckResponse CheckUniformType(spirv_cross::TypeID type);
	void Handle64BitTypes(GLint uniformLocation, ShaderUploadItem& item, ShaderItemInfo& info, int dims);
}