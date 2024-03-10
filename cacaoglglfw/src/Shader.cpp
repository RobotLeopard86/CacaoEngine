#include "Graphics/Shader.hpp"
#include "GLShaderData.hpp"
#include "Core/Assert.hpp"
#include "Core/Log.hpp"

#include "glad/gl.h"
#include "spirv_glsl.hpp"
#include "spirv_cross.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/matrix.hpp"

#include <fstream>
#include <utility>

//For my sanity
#define nd ((GLShaderData*)nativeData)

namespace Cacao {
	Shader::Shader(std::filesystem::path vertex, std::filesystem::path fragment, ShaderSpec spec) {
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
		Shader(vbuf, fbuf, spec);
	}

	Shader::Shader(std::vector<uint32_t> vertex, std::vector<uint32_t> fragment, ShaderSpec spec)
		:compiled(false), bound(false), specification(spec) {
		//Create native data
		nativeData = new GLShaderData();
		
		//Convert SPIR-V to GLSL

		//Create common options
		spirv_cross::CompilerGLSL::Options options;
		options.es = false;
		options.version = 330;
		options.enable_420pack_extension = false;

		//Load vertex shader
		spirv_cross::CompilerGLSL vertGLSL(std::move(vertex));

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

		//Setup Cacao data UBO
		glGenBuffers(1, &(nd->cacaoDataUBO));
		glBindBuffer(GL_UNIFORM_BUFFER, nd->cacaoDataUBO);
		glBufferData(GL_UNIFORM_BUFFER, 3*sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//Link Cacao data UBO
		GLuint cacaoUBOIndex = glGetUniformBlockIndex(program, "CacaoData");
		Asserts::EngineAssert(cacaoUBOIndex != GL_INVALID_INDEX, "Shaders are required to contain the CacaoData uniform block!");
		glUniformBlockBinding(program, cacaoUBOIndex, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, nd->cacaoDataUBO); 

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

	void Shader::UploadData(ShaderUploadData& data) {
		if(!compiled){
            Logging::EngineLog("Cannot upload data to uncompiled shader!", LogLevel::Error);
            return;
        }

		//Create a map for quicker reference later
		std::map<std::string, ShaderItemInfo> foundItems;
		for(ShaderUploadItem item : data) {
			//Attempt to locate item in shader spec
			bool found = false;
			if(!foundItems.contains(item.target)) {
				for(ShaderItemInfo sii : specification){
					foundItems.insert_or_assign(sii.entryName, sii);
					if(sii.entryName.compare(item.target) == 0){
						found = true;
						break;
					}
				}
			}
			if(!found){
				Logging::EngineLog("Can't locate targeted item in shader specification, data upload aborted!", LogLevel::Error);
				return;
			}

			//Obtain uniform location
			GLint uniformLocation = glGetUniformLocation(nd->gpuID, item.target.c_str());
			Asserts::EngineAssert(uniformLocation != -1, "Requested uniform does not exist in shader!");

			//Grab shader item info
			ShaderItemInfo info = foundItems[item.target];

			//For quicker access
			using SpvType = spirv_cross::SPIRType::BaseType;

			//Turn dimensions into single number (easier for uploading)
			int dims = (4*info.size.y)-(4-info.size.x);
			if(info.size.x == 1 && info.size.y < 2){
				Logging::EngineLog("Shaders cannot have data with one column and 2+ rows, data upload aborted!", LogLevel::Error);
				return;
			}
			if(info.size.x > 1 && info.size.y > 1 && info.type != SpvType::Float && info.type != SpvType::Double){
				Logging::EngineLog("Shaders cannot have data with 2+ columns and rows that are not floats or doubles, data upload aborted!", LogLevel::Error);
				return;
			}

			//Get ID of currently bound shader (to restore later)
			GLint currentlyBound;
			glGetIntegerv(GL_CURRENT_PROGRAM, &currentlyBound);

			//Bind shader
			Bind();

			//Attempt to cast data to correct type and upload it
			//it is so annoying that this is how this must be done
			try {
				switch(info.type){
				case SpvType::Boolean:
					switch(dims){
					case 1:
						glUniform1i(uniformLocation, std::any_cast<bool>(item.data));
					case 2:
						glUniform2iv(uniformLocation, glm::value_ptr(std::any_cast<glm::bvec2>(item.data)));
					case 3:
						glUniform3iv(uniformLocation, glm::value_ptr(std::any_cast<glm::bvec3>(item.data)));
					case 4:
						glUniform4iv(uniformLocation, glm::value_ptr(std::any_cast<glm::bvec4>(item.data)));
					}
					break;
				case SpvType::Int:
					switch(dims){
					case 1:
						glUniform1i(uniformLocation, std::any_cast<int>(item.data));
					case 2:
						glUniform2iv(uniformLocation, glm::value_ptr(std::any_cast<glm::ivec2>(item.data)));
					case 3:
						glUniform3iv(uniformLocation, glm::value_ptr(std::any_cast<glm::ivec3>(item.data)));
					case 4:
						glUniform4iv(uniformLocation, glm::value_ptr(std::any_cast<glm::ivec4>(item.data)));
					}
					break;
				case SpvType::Int64:
					switch(dims){
					case 1:
						glUniform1i64ARB(uniformLocation, std::any_cast<int64_t>(item.data));
					case 2:
						glUniform2i64vARB(uniformLocation, glm::value_ptr(std::any_cast<glm::i64vec2>(item.data)));
					case 3:
						glUniform3i64vARB(uniformLocation, glm::value_ptr(std::any_cast<glm::i64vec3>(item.data)));
					case 4:
						glUniform4i64vARB(uniformLocation, glm::value_ptr(std::any_cast<glm::i64vec4>(item.data)));
					}
					break;
				case SpvType::UInt:
					switch(dims){
					case 1:
						glUniform1ui(uniformLocation, std::any_cast<unsigned int>(item.data));
					case 2:
						glUniform2uiv(uniformLocation, glm::value_ptr(std::any_cast<glm::uvec2>(item.data)));
					case 3:
						glUniform3uiv(uniformLocation, glm::value_ptr(std::any_cast<glm::uvec3>(item.data)));
					case 4:
						glUniform4uiv(uniformLocation, glm::value_ptr(std::any_cast<glm::uvec4>(item.data)));
					}
					break;
				case SpvType::UInt64:
					switch(dims){
					case 1:
						glUniform1ui64ARB(uniformLocation, std::any_cast<uint64_t>(item.data));
					case 2:
						glUniform2ui64vARB(uniformLocation, glm::value_ptr(std::any_cast<glm::u64vec2>(item.data)));
					case 3:
						glUniform3ui64vARB(uniformLocation, glm::value_ptr(std::any_cast<glm::u64vec3>(item.data)));
					case 4:
						glUniform4ui64vARB(uniformLocation, glm::value_ptr(std::any_cast<glm::u64vec4>(item.data)));
					}
					break;
				case SpvType::Float:
					switch(dims){
					case 1:
						glUniform1f(uniformLocation, std::any_cast<float>(item.data));
					case 2:
						glUniform2fv(uniformLocation, glm::value_ptr(std::any_cast<glm::vec2>(item.data)));
					case 3:
						glUniform3fv(uniformLocation, glm::value_ptr(std::any_cast<glm::vec3>(item.data)));
					case 4:
						glUniform4fv(uniformLocation, glm::value_ptr(std::any_cast<glm::vec4>(item.data)));
					case 6:
						glUniformMatrix2fv(uniformLocation, glm::value_ptr(std::any_cast<glm::mat2>(item.data)));
					case 7:
						glUniformMatrix2x3fv(uniformLocation, glm::value_ptr(std::any_cast<glm::mat2x3>(item.data)));
					case 8:
						glUniformMatrix2x4fv(uniformLocation, glm::value_ptr(std::any_cast<glm::mat2x4>(item.data)));
					case 10:
						glUniformMatrix3x2fv(uniformLocation, glm::value_ptr(std::any_cast<glm::mat3x2>(item.data)));
					case 11:
						glUniformMatrix3fv(uniformLocation, glm::value_ptr(std::any_cast<glm::mat3>(item.data)));
					case 12:
						glUniformMatrix3x4fv(uniformLocation, glm::value_ptr(std::any_cast<glm::mat3x4>(item.data)));
					case 14:
						glUniformMatrix4x2fv(uniformLocation, glm::value_ptr(std::any_cast<glm::mat4x2>(item.data)));
					case 15:
						glUniformMatrix4x3fv(uniformLocation, glm::value_ptr(std::any_cast<glm::mat4x3>(item.data)));
					case 16:
						glUniformMatrix4fv(uniformLocation, glm::value_ptr(std::any_cast<glm::mat4>(item.data)));
					}
					break;
				case SpvType::Double:
					switch(dims){
					case 1:
						glUniform1d(uniformLocation, std::any_cast<double>(item.data));
					case 2:
						glUniform2dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dvec2>(item.data)));
					case 3:
						glUniform3dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dvec3>(item.data)));
					case 4:
						glUniform4dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dvec4>(item.data)));
					case 6:
						glUniformMatrix2dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dmat2>(item.data)));
					case 7:
						glUniformMatrix2x3dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dmat2x3>(item.data)));
					case 8:
						glUniformMatrix2x4dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dmat2x4>(item.data)));
					case 10:
						glUniformMatrix3x2dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dmat3x2>(item.data)));
					case 11:
						glUniformMatrix3dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dmat3>(item.data)));
					case 12:
						glUniformMatrix3x4dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dmat3x4>(item.data)));
					case 14:
						glUniformMatrix4x2dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dmat4x2>(item.data)));
					case 15:
						glUniformMatrix4x3dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dmat4x3>(item.data)));
					case 16:
						glUniformMatrix4dv(uniformLocation, glm::value_ptr(std::any_cast<glm::dmat4>(item.data)));
					}
					break;
				}
			} catch(const std::bad_cast&) {
				Logging::EngineLog("Failed cast of shader upload value to type specified in target, data upload aborted!");
				return;
			}

			//Restore previous shader
			Unbind();
			glUseProgram(currentlyBound);
		}
	}

	void Shader::UploadCacaoData(glm::mat4 projection, glm::mat4 view, glm::mat4 transform){
		//Bind UBO
		glBindBuffer(GL_UNIFORM_BUFFER, nd->cacaoDataUBO);

		//Upload data
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(transform));

		//Unbind UBO
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}