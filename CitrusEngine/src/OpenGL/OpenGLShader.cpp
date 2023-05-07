#include "OpenGLShader.h"

#include "glad/gl.h"

#include "Core/Log.h"
#include "Core/Assert.h"

#include "glm/gtc/type_ptr.hpp"

namespace CitrusEngine {

    Shader* Shader::CreateShader(std::string vertexSrc, std::string fragmentSrc){
        return new OpenGLShader(vertexSrc, fragmentSrc);
    }

    OpenGLShader::OpenGLShader(std::string vertexSrc, std::string fragmentSrc){
        compiled = false;
        bound = false;
        vertexShader = vertexSrc;
        fragmentShader = fragmentSrc;
    }

    void OpenGLShader::Compile() {
        if(compiled){
            Logging::EngineLog(LogLevel::Warn, "Recompiling already compiled shader...");
        }

        //Create vertex shader base
        GLuint compiledVertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar* vertexSrc = vertexShader.c_str();

        //Compile vertex shader
        glShaderSource(compiledVertexShader, 1, &vertexSrc, 0);
        glCompileShader(compiledVertexShader);

        //Confirm vertex shader compilation
        GLint vertexCompileStatus;
        glGetShaderiv(compiledVertexShader, GL_COMPILE_STATUS, &vertexCompileStatus);
        if(vertexCompileStatus == GL_FALSE){
            //Get compilation log
            GLint maxLen = 0;
            glGetShaderiv(compiledVertexShader, GL_INFO_LOG_LENGTH, &maxLen);
            std::vector<GLchar> infoLog(maxLen);
            glGetShaderInfoLog(compiledVertexShader, maxLen, &maxLen, &infoLog[0]);
            //Clean up resources
            glDeleteShader(compiledVertexShader);
            //Log error
            Logging::EngineLog(LogLevel::Error, std::string("Vertex shader compilation failure: ") + infoLog.data());
            return;
        }

        //Create fragment shader base
        GLuint compiledFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar* fragmentSrc = fragmentShader.c_str();

        //Compile fragment shader
        glShaderSource(compiledFragmentShader, 1, &fragmentSrc, 0);
        glCompileShader(compiledFragmentShader);

        //Confirm fragment shader compilation
        GLint fragmentCompileStatus;
        glGetShaderiv(compiledFragmentShader, GL_COMPILE_STATUS, &fragmentCompileStatus);
        if(fragmentCompileStatus == GL_FALSE){
            //Get compilation log
            GLint maxLen = 0;
            glGetShaderiv(compiledFragmentShader, GL_INFO_LOG_LENGTH, &maxLen);
            std::vector<GLchar> infoLog(maxLen);
            glGetShaderInfoLog(compiledFragmentShader, maxLen, &maxLen, &infoLog[0]);
            //Clean up resources
            glDeleteShader(compiledVertexShader);
            glDeleteShader(compiledFragmentShader);
            //Log error
            Logging::EngineLog(LogLevel::Error, std::string("Fragment shader compilation failure: ") + infoLog.data());
            return;
        }

        //Create shader program
        GLuint program = glCreateProgram();
        
        //Attach compiled shaders to program
        glAttachShader(program, compiledVertexShader);
        glAttachShader(program, compiledFragmentShader);

        //Link shader program
        glLinkProgram(program);

        //Confirm shader program linking
        GLint linkStatus;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if(linkStatus == GL_FALSE){
            //Get linking log
            GLint maxLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLen);
            std::vector<GLchar> infoLog(maxLen);
            glGetProgramInfoLog(program, maxLen, &maxLen, &infoLog[0]);
            //Clean up resources
            glDeleteProgram(program);
            glDeleteShader(compiledVertexShader);
            glDeleteShader(compiledFragmentShader);
            //Log error
            Logging::EngineLog(LogLevel::Error, std::string("Shader program linking failure: ") + infoLog.data());
            return;
        }

        //Detach and delete linked shaders
        glDetachShader(program, compiledFragmentShader);
        glDetachShader(program, compiledVertexShader);
        glDeleteShader(compiledVertexShader);
        glDeleteShader(compiledFragmentShader);

        compiledForm = program;
        compiled = true;
    }

    void OpenGLShader::Bind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot bind uncompiled shader!");
            return;
        }
        if(bound){
            Logging::EngineLog(LogLevel::Error, "Cannot bind already bound shader!");
            return;
        }
        glUseProgram(compiledForm);
        bound = true;
    }

    void OpenGLShader::Unbind(){
        if(!compiled){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind uncompiled shader!");
            return;
        }
        if(!bound){
            Logging::EngineLog(LogLevel::Error, "Cannot unbind unbound shader!");
            return;
        }

        //Preserve shader program so it doesn't need to be recompiled
        uint32_t shaderProgram = compiledForm;
        glDeleteProgram(compiledForm);
        compiledForm = shaderProgram;

        bound = false;
    }

    void OpenGLShader::UploadUniformMat4(std::string uniform, glm::mat4 value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if(uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
	}

	void OpenGLShader::UploadUniformMat3(std::string uniform, glm::mat3 value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if(uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
	}

    void OpenGLShader::UploadUniformBoolean(std::string uniform, bool value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if (uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniform1i(uniformLocation, value);
	}

    void OpenGLShader::UploadUniformFloat(std::string uniform, float value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if(uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniform1f(uniformLocation, value);
	}

	void OpenGLShader::UploadUniformFloat2(std::string uniform, glm::vec2 value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if(uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniform2f(uniformLocation, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(std::string uniform, glm::vec3 value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if(uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniform3f(uniformLocation, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(std::string uniform, glm::vec4 value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if(uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniform4f(uniformLocation, value.x, value.y, value.z, value.w);
	}

    void OpenGLShader::UploadUniformInt(std::string uniform, int value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if(uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniform1i(uniformLocation, value);
	}

	void OpenGLShader::UploadUniformInt2(std::string uniform, glm::i32vec2 value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if(uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniform2i(uniformLocation, value.x, value.y);
	}

	void OpenGLShader::UploadUniformInt3(std::string uniform, glm::i32vec3 value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if(uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniform3i(uniformLocation, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformInt4(std::string uniform, glm::i32vec4 value) {
		GLint uniformLocation = glGetUniformLocation(compiledForm, uniform.c_str());
		if(uniformLocation == -1) {
			Asserts::EngineAssert(false, "Cannot upload data to nonexistent uniform!");
			return;
		}
		glUniform4i(uniformLocation, value.x, value.y, value.z, value.w);
	}
}