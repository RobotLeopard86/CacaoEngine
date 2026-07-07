#include "OpenGLShader.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "OpenGLModule.hpp"
#include "CommandBufferCast.hpp"

namespace Cacao {
	void OpenGLShaderImpl::Bake(bool& success) {
		success = true;
	}

	void OpenGLShaderImpl::Discard() {}

	Shader::Impl* OpenGLModule::ConfigureShader() {
		return new OpenGLShaderImpl();
	}
}