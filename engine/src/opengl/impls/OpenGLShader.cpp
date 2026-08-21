#include "OpenGLShader.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "OpenGLModule.hpp"
#include "CommandBufferCast.hpp"
#include "Targetgen.hpp"

#include "glad/gl.h"

namespace Cacao {
	void OpenGLShaderImpl::Bake(bool& success) {
		//Open-GL specific stuff needs to be on the GPU thread
		std::unique_ptr<OpenGLCommandBuffer> cmd = CBCast<OpenGLCommandBuffer>(CommandBuffer::Create());
		cmd->AddTask([this, &success]() {
			//Convert Slang IR to GLSL
			libcacaoasset::Shader shdr = {};
			shdr.irCode = irBuffer;
			shdr.descriptor = descriptor;
			GLSL code = GenerateGLSL(shdr, false);
			vertexGLSL = code.vertex;
			fragmentGLSL = code.fragment;

			//Compile vertex stage
			GLuint vertexStage = glCreateShader(GL_VERTEX_SHADER);
			const GLchar* vsrc = vertexGLSL.c_str();
			glShaderSource(vertexStage, 1, &vsrc, 0);
			glCompileShader(vertexStage);

			//Check success
			GLint status;
			glGetShaderiv(vertexStage, GL_COMPILE_STATUS, &status);
			if(status == GL_FALSE) {
				//Obtain error
				GLint len = 0;
				glGetShaderiv(vertexStage, GL_INFO_LOG_LENGTH, &len);
				std::string log;
				log.resize(len);
				glGetShaderInfoLog(vertexStage, len, &len, log.data());

				//Clean up resources
				glDeleteShader(vertexStage);

				//Throw error
				Check<ExternalException>(false, std::format("Failed to compile shader vertex stage: {}", log));
			}

			//Compile fragment stage
			GLuint fragmentStage = glCreateShader(GL_FRAGMENT_SHADER);
			const GLchar* fsrc = fragmentGLSL.c_str();
			glShaderSource(fragmentStage, 1, &fsrc, 0);
			glCompileShader(fragmentStage);

			//Check success
			glGetShaderiv(fragmentStage, GL_COMPILE_STATUS, &status);
			if(status == GL_FALSE) {
				//Obtain error
				GLint len = 0;
				glGetShaderiv(fragmentStage, GL_INFO_LOG_LENGTH, &len);
				std::string log;
				log.resize(len);
				glGetShaderInfoLog(fragmentStage, len, &len, log.data());

				//Clean up resources
				glDeleteShader(fragmentStage);
				glDeleteShader(vertexStage);

				//Throw error
				Check<ExternalException>(false, std::format("Failed to compile shader fragment stage: {}", log));
			}

			//Build shader program
			program = glCreateProgram();
			glAttachShader(program, vertexStage);
			glAttachShader(program, fragmentStage);
			glLinkProgram(program);

			//Check success
			glGetProgramiv(program, GL_LINK_STATUS, &status);
			if(status == GL_FALSE) {
				//Obtain error
				GLint len = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
				std::string log;
				log.resize(len);
				glGetProgramInfoLog(program, len, &len, log.data());

				//Clean up resources
				glDeleteProgram(program);
				glDeleteShader(fragmentStage);
				glDeleteShader(vertexStage);

				//Throw error
				Check<ExternalException>(false, std::format("Failed to link shader program: {}", log));
			}

			//Free shader stages (no longer needed)
			glDetachShader(program, vertexStage);
			glDetachShader(program, fragmentStage);
			glDeleteShader(vertexStage);
			glDeleteShader(fragmentStage);

			//Bind globals UBO to program
			GLuint globalDataIndex = glGetUniformBlockIndex(program, "Cacao_GlobalData_std140");
			Check<ExternalException>(globalDataIndex != GL_INVALID_INDEX, "Failed to locate engine globals data uniform block!");
			glUniformBlockBinding(program, globalDataIndex, 0);

			//Create and bind material data uniform
			if(descriptor.uniformParams.size() > 0) {
				//Create buffer
				glGenBuffers(1, &ubo);

				//Bind it to the program
				GLuint materialDataIndex = glGetUniformBlockIndex(program, "MaterialParameters_std140");
				Check<ExternalException>(materialDataIndex != GL_INVALID_INDEX, "Failed to locate material parameters uniform block!");
				uboBinding = (gl->shaderBindingCounter)++;
				glUniformBlockBinding(program, materialDataIndex, uboBinding);
			}

			//Get uniform locations
			transformUloc = glGetUniformLocation(program, "transform.transformMatrix");
			hasTransform = (transformUloc != -1);
			normalMatrixUloc = glGetUniformLocation(program, "transform.normalMatrix");
			Check<ExternalException>(!hasTransform || (hasTransform && normalMatrixUloc != -1), "Failed to locate transformation normal matrix uniform!");
			handednessUloc = glGetUniformLocation(program, "transform.handedness");
			Check<ExternalException>(!hasTransform || (hasTransform && handednessUloc != -1), "Failed to locate transformation handedness uniform!");
			renderModeUloc = glGetUniformLocation(program, "renderMode.mode");
			Check<ExternalException>(renderModeUloc != -1, "Failed to locate render mode uniform!");

			success = true;
		});
		GPUManager::Get().Submit(std::move(cmd)).get();
	}

	void OpenGLShaderImpl::Discard() {
		std::unique_ptr<OpenGLCommandBuffer> cmd = CBCast<OpenGLCommandBuffer>(CommandBuffer::Create());
		cmd->AddTask([this]() {
			glDeleteProgram(program);
		});
		GPUManager::Get().Submit(std::move(cmd)).get();
	}

	void OpenGLShaderImpl::Bind(bool transparent) {
		//Bind shader object
		glUseProgram(program);

		//Configure blending and depth
		glEnable(GL_BLEND);
		if(customSettings) {
			glBlendFunc((customSettings && customSettings->blendUseOne) ? GL_ONE : GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

		//Bind the globals UBO
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, gl->globalsUBO);

		//Bind our UBO
		if(ubo != UINT32_MAX) glBindBufferBase(GL_UNIFORM_BUFFER, uboBinding, ubo);
	}

	Shader::Impl* OpenGLModule::ConfigureShader() {
		return new OpenGLShaderImpl();
	}
}