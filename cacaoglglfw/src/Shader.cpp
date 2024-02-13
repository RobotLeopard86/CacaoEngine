#include "Graphics/Shader.hpp"
#include "GLShaderData.hpp"
#include "Core/Assert.hpp"
#include "Core/Log.hpp"

#include "glad/gl.h"
#include "spirv_glsl.hpp"

#include <fstream>
#include <utility>

//For my sanity
#define nd ((GLShaderData*)nativeData)

namespace Cacao {
	Shader::Shader(std::filesystem::path vertex, std::filesystem::path fragment) {
		//Validate that these paths exist
		Asserts::EngineAssert(std::filesystem::exists(vertex), "Cannot create a shader from a non-existent file!");
		Asserts::EngineAssert(std::filesystem::exists(fragment), "Cannot create a shader from a non-existent file!");

		//Load SPIR-V code

		//Open file streams
		//Start at end of file to easily determine necessary buffer size
		std::ifstream vertStream(vertex, std::ios::ate | std::ios::binary);
		Asserts::EngineAssert(vertStream.is_open(), "Cannot open vertex shader code file!");
		std::ifstream fragStream(vertex, std::ios::ate | std::ios::binary);
		Asserts::EngineAssert(fragStream.is_open(), "Cannot open fragment shader code file!");

		//Allocate data buffers
		size_t vfs = (size_t) vertStream.tellg();
		std::vector<uint32_t> vbuf(vfs);
		size_t ffs = (size_t) fragStream.tellg();
		std::vector<uint32_t> fbuf(ffs);

		//Read data into buffers
		vertStream.seekg(0);
		vertStream.read(reinterpret_cast<char*>(vbuf.data()), vfs);
		fragStream.seekg(0);
		fragStream.read(reinterpret_cast<char*>(vbuf.data()), ffs);

		//Close streams
		vertStream.close();
		fragStream.close();

		//Construct shader with loaded code
		Shader(vbuf, fbuf);
	}

	Shader::Shader(std::vector<uint32_t> vertex, std::vector<uint32_t> fragment)
		:compiled(false), bound(false) {
		//Create native data
		nativeData = new GLShaderData();
		
		//Convert SPIR-V to GLSL

		//Create common options
		spirv_cross::CompilerGLSL::Options options;
		options.es = false;
		options.version = 330;
		options.emit_push_constant_as_uniform_buffer = true;
		options.enable_420pack_extension = false;

		//Load vertex shader
		spirv_cross::CompilerGLSL vertGLSL(std::move(vertex));
		spirv_cross::ShaderResources vertRes = vertGLSL.get_shader_resources();

		//Compile vertex shader to GLSL
		vertGLSL.set_common_options(options);
		nd->vertexCode = vertGLSL.compile();

		//Load fragment shader
		spirv_cross::CompilerGLSL fragGLSL(std::move(fragment));
		spirv_cross::ShaderResources fragRes = fragGLSL.get_shader_resources();

		//Remove image decorations
		for(auto& img : fragRes.sampled_images){
			fragGLSL.unset_decoration(img.id, spv::DecorationDescriptorSet);
		}

		//Compile fragment shader to GLSL
		fragGLSL.set_common_options(options);
		nd->fragmentCode = fragGLSL.compile();
	}

	void Shader::Compile(){
		if(compiled){
            Logging::EngineLog("Not compiling already compiled shader!", LogLevel::Warn);
			return;
        }

        //Create vertex shader base
        GLuint compiledVertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar* vertexSrc = nd->vertexCode.c_str();

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
            Logging::EngineLog(std::string("Vertex shader compilation failure: ") + infoLog.data(), LogLevel::Error);
            return;
        }

        //Create fragment shader base
        GLuint compiledFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar* fragmentSrc = nd->fragmentCode.c_str();

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
            Logging::EngineLog(std::string("Fragment shader compilation failure: ") + infoLog.data(), LogLevel::Error);
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
            Logging::EngineLog(std::string("Shader program linking failure: ") + infoLog.data(), LogLevel::Error);
            return;
        }

        //Detach and delete linked shaders
        glDetachShader(program, compiledFragmentShader);
        glDetachShader(program, compiledVertexShader);
        glDeleteShader(compiledVertexShader);
        glDeleteShader(compiledFragmentShader);

		//Configure rendering UBO
		unsigned int ubi = glGetUniformBlockIndex(program, "CacaoData");
		Asserts::EngineAssert(ubi != GL_INVALID_INDEX, "Cacao Engine shader does not contain the CacaoData uniform block!");
		glUniformBlockBinding(program, ubi, 0);

        nd->gpuID = program;
        compiled = true;
	}

	void Shader::Release(){
        if(!compiled){
            Logging::EngineLog("Cannot release uncompiled shader!", LogLevel::Error);
            return;
        }
        if(bound){
            Logging::EngineLog("Cannot release bound shader!", LogLevel::Error);
            return;
        }
        glDeleteProgram(nd->gpuID);
        compiled = false;
    }

    void Shader::Bind(){
        if(!compiled){
            Logging::EngineLog("Cannot bind uncompiled shader!", LogLevel::Error);
            return;
        }
        if(bound){
            Logging::EngineLog("Cannot bind already bound shader!", LogLevel::Error);
            return;
        }
        glUseProgram(nd->gpuID);
        bound = true;
    }

    void Shader::Unbind(){
        if(!compiled){
            Logging::EngineLog("Cannot unbind uncompiled shader!", LogLevel::Error);
            return;
        }
        if(!bound){
            Logging::EngineLog("Cannot unbind unbound shader!", LogLevel::Error);
            return;
        }

        //Clear current program
        glUseProgram(0);

        bound = false;
    }
}