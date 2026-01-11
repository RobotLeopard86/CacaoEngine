#include "OpenGLShader.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "OpenGLModule.hpp"
#include "CommandBufferCast.hpp"

namespace Cacao {
	void OpenGLShaderImpl::Realize(bool& success) {
		success = true;
	}

	void OpenGLShaderImpl::DropRealized() {}

	Shader::Impl* OpenGLModule::ConfigureShader() {
		return new OpenGLShaderImpl();
	}
}