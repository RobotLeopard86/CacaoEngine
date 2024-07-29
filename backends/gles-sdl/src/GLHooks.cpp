#include "GLHooks.hpp"

namespace Cacao {
	UniformTypeCheckResponse CheckUniformType(spirv_cross::TypeID type) {
		UniformTypeCheckResponse retval = {};
		retval.ok = false;
		retval.err = "";

		if(type == SpvType::Double) {
			retval.err = "OpenGL ES does not support double-precision types for shaders!";
			return retval;
		} else if(type == SpvType::Int64 || type == SpvType::UInt64) {
			retval.err = "OpenGL ES does not support 64-bit types for shaders!";
			return retval;
		}

		retval.ok = true;
		return retval;
	}

	//Does nothing because the above check means this will never get called, we just can't have those functions in the compilation unit
	void Handle64BitTypes(GLint, ShaderUploadItem& _, ShaderItemInfo& __, int) {}

	void ConfigureSPIRV(spirv_cross::CompilerGLSL::Options* opts) {
		opts->version = 300;
		opts->es = true;
	}
}