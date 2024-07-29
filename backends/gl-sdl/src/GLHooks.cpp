#include "GLHooks.hpp"

#include "glm/gtc/type_ptr.hpp"

namespace Cacao {
	UniformTypeCheckResponse CheckUniformType(spirv_cross::TypeID type) {
		return {.ok = true, .err = ""};
	}

	//Does nothing because the above check means this will nevere get called, we just can't have those functions in the compilation unit
	void Handle64BitTypes(GLint uniformLocation, ShaderUploadItem& item, ShaderItemInfo& info, int dims) {
		switch(info.type) {
			case SpvType::Double:
				switch(dims) {
					case 1:
						glUniform1d(uniformLocation, std::any_cast<double>(item.data));
						break;
					case 2:
						glUniform2dv(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::dvec2>(item.data)));
						break;
					case 3:
						glUniform3dv(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::dvec3>(item.data)));
						break;
					case 4:
						glUniform4dv(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::dvec4>(item.data)));
						break;
					case 6:
						glUniformMatrix2dv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::dmat2>(item.data)));
						break;
					case 7:
						glUniformMatrix2x3dv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::dmat2x3>(item.data)));
						break;
					case 8:
						glUniformMatrix2x4dv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::dmat2x4>(item.data)));
						break;
					case 10:
						glUniformMatrix3x2dv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::dmat3x2>(item.data)));
						break;
					case 11:
						glUniformMatrix3dv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::dmat3>(item.data)));
						break;
					case 12:
						glUniformMatrix3x4dv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::dmat3x4>(item.data)));
						break;
					case 14:
						glUniformMatrix4x2dv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::dmat4x2>(item.data)));
						break;
					case 15:
						glUniformMatrix4x3dv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::dmat4x3>(item.data)));
						break;
					case 16:
						glUniformMatrix4dv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::dmat4>(item.data)));
						break;
				}
				break;
			case SpvType::Int64:
				switch(dims) {
					case 1:
						glUniform1i64ARB(uniformLocation, std::any_cast<int64_t>(item.data));
						break;
					case 2:
						glUniform2i64vARB(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::i64vec2>(item.data)));
						break;
					case 3:
						glUniform3i64vARB(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::i64vec3>(item.data)));
						break;
					case 4:
						glUniform4i64vARB(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::i64vec4>(item.data)));
						break;
				}
				break;
			case SpvType::UInt64:
				switch(dims) {
					case 1:
						glUniform1ui64ARB(uniformLocation, std::any_cast<uint64_t>(item.data));
						break;
					case 2:
						glUniform2ui64vARB(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::u64vec2>(item.data)));
						break;
					case 3:
						glUniform3ui64vARB(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::u64vec3>(item.data)));
						break;
					case 4:
						glUniform4ui64vARB(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::u64vec4>(item.data)));
						break;
				}
				break;
		}
	}

	void ConfigureSPIRV(spirv_cross::CompilerGLSL::Options* opts) {
		opts->version = 410;
		opts->es = false;
	}
}