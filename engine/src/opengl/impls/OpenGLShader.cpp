#include "OpenGLShader.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "OpenGLModule.hpp"
#include "CommandBufferCast.hpp"
#include "glad/gl.h"

namespace Cacao {
	void OpenGLShaderImpl::Bake(bool& success) {
		success = true;
	}

	void OpenGLShaderImpl::Discard() {}

	void OpenGLShaderImpl::Bind(bool transparent) {
		//Bind shader object
		glUseProgram(program);

		//Configure blending and depth
		glEnable(GL_BLEND);
		if(customSettings) {
			glBlendFunc(customSettings->blendUseSrc ? GL_SRC_ALPHA : GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			if(customSettings->depth != CustomCompileSettings::Depth::Off) {
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(customSettings->depth == CustomCompileSettings::Depth::Less ? GL_LESS : GL_LEQUAL);
			} else {
				glDisable(GL_DEPTH_TEST);
			}
		}
		if(!transparent) {
			glDepthMask(GL_TRUE);
		} else {
			glDepthMask(GL_FALSE);
		}

		//Bind our UBO
		if(ubo != UINT32_MAX) glBindBufferBase(GL_UNIFORM_BUFFER, uboBinding, ubo);
	}

	Shader::Impl* OpenGLModule::ConfigureShader() {
		return new OpenGLShaderImpl();
	}
}