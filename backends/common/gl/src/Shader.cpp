#include "Graphics/Shader.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "GLShaderData.hpp"
#include "Core/Assert.hpp"
#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "GLUtils.hpp"
#include "GLHooks.hpp"

#include "GLHeaders.hpp"
#include "spirv_glsl.hpp"
#include "spirv_cross.hpp"
#include "spirv_parser.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/matrix.hpp"

#include <cstdio>
#include <cstring>
#include <utility>
#include <future>
#include <iostream>

namespace Cacao {
	//A required static member initialization
	GLuint Shader::ShaderData::uboIndexCounter = 0;

	Shader::Shader(std::string vertexPath, std::string fragmentPath, ShaderSpec spec)
	  : Asset(false), bound(false), specification(spec) {
		//Validate that these paths exist
		CheckException(std::filesystem::exists(vertexPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create a shader from a non-existent vertex shader file!")
		CheckException(std::filesystem::exists(fragmentPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create a shader from a non-existent fragment shader file!")

		//Load SPIR-V code

		//Open file streams
		FILE* vf = fopen(vertexPath.c_str(), "rb");
		CheckException(vf, Exception::GetExceptionCodeFromMeaning("FileOpenFailure"), "Failed to open vertex shader file!")
		FILE* ff = fopen(fragmentPath.c_str(), "rb");
		CheckException(ff, Exception::GetExceptionCodeFromMeaning("FileOpenFailure"), "Failed to open fragment shader file!")

		//Get size of shader files
		fseek(vf, 0, SEEK_END);
		long vlen = ftell(vf) / sizeof(uint32_t);
		rewind(vf);
		fseek(ff, 0, SEEK_END);
		long flen = ftell(ff) / sizeof(uint32_t);
		rewind(ff);

		//Load shader data into vectors
		std::vector<uint32_t> vbuf(vlen);
		std::vector<uint32_t> fbuf(flen);
		if(fread(vbuf.data(), sizeof(uint32_t), vlen, vf) != size_t(vlen)) {
			//Clean up file streams before exception throw
			fclose(vf);
			fclose(ff);
			CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to read vertex shader data from file!")
		}
		if(fread(fbuf.data(), sizeof(uint32_t), flen, ff) != size_t(flen)) {
			//Clean up file streams before exception throw
			fclose(vf);
			fclose(ff);
			CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to read fragment shader data from file!")
		}

		//Close file streams
		fclose(vf);
		fclose(ff);

		//Create native data
		nativeData.reset(new ShaderData());

		//Convert SPIR-V to GLSL

		//Create common options
		spirv_cross::CompilerGLSL::Options options;
		options.es = true;
		options.version = 300;
		options.enable_420pack_extension = false;

		//Parse SPIR-V IR
		spirv_cross::Parser vertParse(std::move(vbuf));
		vertParse.parse();
		spirv_cross::Parser fragParse(std::move(fbuf));
		fragParse.parse();

		//Load vertex shader
		spirv_cross::CompilerGLSL vertGLSL(std::move(vertParse.get_parsed_ir()));

		//Compile vertex shader to GLSL
		vertGLSL.set_common_options(options);
		nativeData->vertexCode = vertGLSL.compile();

		//Load fragment shader
		spirv_cross::CompilerGLSL fragGLSL(std::move(fragParse.get_parsed_ir()));
		spirv_cross::ShaderResources fragRes = fragGLSL.get_shader_resources();

		//Remove image decorations
		for(auto& img : fragRes.sampled_images) {
			fragGLSL.unset_decoration(img.id, spv::DecorationDescriptorSet);
		}

		//Compile fragment shader to GLSL
		fragGLSL.set_common_options(options);
		nativeData->fragmentCode = fragGLSL.compile();
	}

	Shader::Shader(std::vector<uint32_t>& vertex, std::vector<uint32_t>& fragment, ShaderSpec spec)
	  : Asset(false), bound(false), specification(spec) {
		//Create native data
		nativeData.reset(new ShaderData());

		//Convert SPIR-V to GLSL

		//Create common options
		spirv_cross::CompilerGLSL::Options options;
		options.es = true;
		options.version = 300;
		options.enable_420pack_extension = false;

		//Parse SPIR-V IR
		spirv_cross::Parser vertParse(std::move(vertex));
		vertParse.parse();
		spirv_cross::Parser fragParse(std::move(fragment));
		fragParse.parse();

		//Load vertex shader
		spirv_cross::CompilerGLSL vertGLSL(std::move(vertParse.get_parsed_ir()));

		//Compile vertex shader to GLSL
		vertGLSL.set_common_options(options);
		nativeData->vertexCode = vertGLSL.compile();

		//Load fragment shader
		spirv_cross::CompilerGLSL fragGLSL(std::move(fragParse.get_parsed_ir()));
		spirv_cross::ShaderResources fragRes = fragGLSL.get_shader_resources();

		//Remove image decorations
		for(auto& img : fragRes.sampled_images) {
			fragGLSL.unset_decoration(img.id, spv::DecorationDescriptorSet);
		}

		//Compile fragment shader to GLSL
		fragGLSL.set_common_options(options);
		nativeData->fragmentCode = fragGLSL.compile();
	}

	std::shared_future<void> Shader::Compile() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Invoke OpenGL (ES) on the main thread
			return InvokeGL([this]() {
				this->Compile();
			});
		}
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled shader!");

		//Create vertex shader base
		GLuint compiledVertexShader = glCreateShader(GL_VERTEX_SHADER);
		const GLchar* vertexSrc = nativeData->vertexCode.c_str();

		//Compile vertex shader
		glShaderSource(compiledVertexShader, 1, &vertexSrc, 0);
		glCompileShader(compiledVertexShader);

		//Confirm vertex shader compilation
		GLint vertexCompileStatus;
		glGetShaderiv(compiledVertexShader, GL_COMPILE_STATUS, &vertexCompileStatus);
		if(vertexCompileStatus == GL_FALSE) {
			//Get compilation log
			GLint maxLen = 0;
			glGetShaderiv(compiledVertexShader, GL_INFO_LOG_LENGTH, &maxLen);
			std::vector<GLchar> infoLog(maxLen);
			glGetShaderInfoLog(compiledVertexShader, maxLen, &maxLen, &infoLog[0]);
			//Clean up resources
			glDeleteShader(compiledVertexShader);
			//Throw exception
			CheckException(false, Exception::GetExceptionCodeFromMeaning("GLESError"), std::string("Vertex shader compilation failure: ") + infoLog.data());
			return {};
		}

		//Create fragment shader base
		GLuint compiledFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		const GLchar* fragmentSrc = nativeData->fragmentCode.c_str();

		//Compile fragment shader
		glShaderSource(compiledFragmentShader, 1, &fragmentSrc, 0);
		glCompileShader(compiledFragmentShader);

		//Confirm fragment shader compilation
		GLint fragmentCompileStatus;
		glGetShaderiv(compiledFragmentShader, GL_COMPILE_STATUS, &fragmentCompileStatus);
		if(fragmentCompileStatus == GL_FALSE) {
			//Get compilation log
			GLint maxLen = 0;
			glGetShaderiv(compiledFragmentShader, GL_INFO_LOG_LENGTH, &maxLen);
			std::vector<GLchar> infoLog(maxLen);
			glGetShaderInfoLog(compiledFragmentShader, maxLen, &maxLen, &infoLog[0]);
			//Clean up resources
			glDeleteShader(compiledVertexShader);
			glDeleteShader(compiledFragmentShader);
			//Throw exception
			CheckException(false, Exception::GetExceptionCodeFromMeaning("GLESError"), std::string("Fragment shader compilation failure: ") + infoLog.data());
			return {};
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
		if(linkStatus == GL_FALSE) {
			//Get linking log
			GLint maxLen = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLen);
			std::vector<GLchar> infoLog(maxLen);
			glGetProgramInfoLog(program, maxLen, &maxLen, &infoLog[0]);
			//Clean up resources
			glDeleteProgram(program);
			glDeleteShader(compiledVertexShader);
			glDeleteShader(compiledFragmentShader);
			//Throw exception
			CheckException(false, Exception::GetExceptionCodeFromMeaning("GLESError"), std::string("Shader linking failure: ") + infoLog.data());
			return {};
		}

		//Detach and delete linked shaders
		glDetachShader(program, compiledFragmentShader);
		glDetachShader(program, compiledVertexShader);
		glDeleteShader(compiledVertexShader);
		glDeleteShader(compiledFragmentShader);

		//Setup Cacao data UBO
		glGenBuffers(1, &(nativeData->cacaoDataUBO));
		glBindBuffer(GL_UNIFORM_BUFFER, nativeData->cacaoDataUBO);
		glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//Link Cacao data UBO
		GLuint cacaoUBOIndex = glGetUniformBlockIndex(program, "CacaoData");
		CheckException(cacaoUBOIndex != GL_INVALID_INDEX, Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "Shader does not contain the CacaoData uniform block!")
		glUniformBlockBinding(program, cacaoUBOIndex, ShaderData::uboIndexCounter);
		glBindBufferBase(GL_UNIFORM_BUFFER, ShaderData::uboIndexCounter, nativeData->cacaoDataUBO);

		//Increment UBO index counter
		ShaderData::uboIndexCounter++;

		//Set GPU ID and compiled values
		nativeData->gpuID = program;
		compiled = true;

		//Return an empty future
		return {};
	}

	void Shader::Release() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Try to invoke OpenGL (ES) and throw any exceptions back to the initial caller
			try {
				InvokeGL([this]() {
					this->Release();
				}).get();
				return;
			} catch(...) {
				std::rethrow_exception(std::current_exception());
			}
		}
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled shader!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot release bound shader!");

		glDeleteProgram(nativeData->gpuID);
		compiled = false;
	}

	void Shader::Bind() {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot bind shader in non-rendering thread!")
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled shader!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound shader!");

		glUseProgram(nativeData->gpuID);
		bound = true;
	}

	void Shader::Unbind() {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetThreadID(), Exception::GetExceptionCodeFromMeaning("RenderThread"), "Cannot unbind shader in non-rendering thread!")
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot unbind uncompiled shader!");
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound shader!");

		//Clear current program
		glUseProgram(0);

		bound = false;
	}

	void Shader::UploadData(ShaderUploadData& data) {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Invoke OpenGL (ES) on the main thread
			InvokeGL([this, data]() {
				this->UploadData(const_cast<ShaderUploadData&>(data));
			});
			return;
		}
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot upload data to uncompiled shader!")

		//Create a map for quicker reference later
		std::map<std::string, ShaderItemInfo> foundItems;
		for(ShaderUploadItem& item : data) {
			//Attempt to locate item in shader spec
			bool found = false;
			if(!foundItems.contains(item.target)) {
				for(ShaderItemInfo sii : specification) {
					foundItems.insert_or_assign(sii.entryName, sii);
					if(sii.entryName.compare(item.target) == 0) {
						found = true;
						break;
					}
				}
			}
			CheckException(found, Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Can't locate targeted item in shader specification!")

			//Grab shader item info
			ShaderItemInfo info = foundItems[item.target];

			//Obtain uniform location
			std::stringstream ulocPath;
			ulocPath << (info.type == SpvType::SampledImage ? "" : "shader.") << item.target;
			GLint uniformLocation = glGetUniformLocation(nativeData->gpuID, ulocPath.str().c_str());
			CheckException(uniformLocation != -1, Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "Shader does not contain the requested uniform!")

			//Confirm that spec does not contain types that aren't supported
			UniformTypeCheckResponse res = CheckUniformType(info.type);
			CheckException(res.ok, Exception::GetExceptionCodeFromMeaning("UnsupportedType"), res.err)

			//Turn dimensions into single number (easier for uploading)
			int dims = (4 * info.size.y) - (4 - info.size.x);
			CheckException(!(info.size.x == 1 && info.size.y >= 2), Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Shaders cannot have data with one column and 2+ rows!")
			CheckException(!(info.size.x > 1 && info.size.y > 1 && info.type != SpvType::Float && info.type != SpvType::Double), Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Shaders cannot have data with 2+ columns and rows that are not floats or doubles!")

			//Get ID of currently bound shader (to restore later)
			//Only do this if we are not currently bound
			GLint currentlyBound = -1;
			if(!bound) {
				glGetIntegerv(GL_CURRENT_PROGRAM, &currentlyBound);

				//Bind shader
				Bind();
			}

			int imageSlotCounter = 0;

			//Attempt to cast data to correct type and upload it
			//It is so annoying that this is how this must be done
			try {
				switch(info.type) {
					case SpvType::Boolean:
						switch(dims) {
							case 1:
								glUniform1i(uniformLocation, std::any_cast<bool>(item.data));
								break;
							case 2:
								glUniform2iv(uniformLocation, 1, glm::value_ptr<int>(glm::ivec2(std::any_cast<glm::ivec2>(item.data))));
								break;
							case 3:
								glUniform3iv(uniformLocation, 1, glm::value_ptr<int>(glm::ivec3(std::any_cast<glm::ivec3>(item.data))));
								break;
							case 4:
								glUniform4iv(uniformLocation, 1, glm::value_ptr<int>(glm::ivec4(std::any_cast<glm::ivec4>(item.data))));
								break;
						}
						break;
					case SpvType::Int:
						switch(dims) {
							case 1:
								glUniform1i(uniformLocation, std::any_cast<int>(item.data));
								break;
							case 2:
								glUniform2iv(uniformLocation, 1, glm::value_ptr<int>(std::any_cast<glm::ivec2>(item.data)));
								break;
							case 3:
								glUniform3iv(uniformLocation, 1, glm::value_ptr<int>(std::any_cast<glm::ivec3>(item.data)));
								break;
							case 4:
								glUniform4iv(uniformLocation, 1, glm::value_ptr<int>(std::any_cast<glm::ivec4>(item.data)));
								break;
						}
						break;
					case SpvType::SampledImage:
						//Confirm valid dimensions
						CheckException(dims == 1, Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Shaders cannot have arrays or matrices of textures!")

						//Bind texture to the next available slot
						//Done in its own scope to avoid compiler error
						if(item.data.type() == typeid(Texture2D*)) {
							Texture2D* tex = std::any_cast<Texture2D*>(item.data);
							tex->Bind(imageSlotCounter);
						} else if(item.data.type() == typeid(Cubemap*)) {
							Cubemap* tex = std::any_cast<Cubemap*>(item.data);
							tex->Bind(imageSlotCounter);
						} else {
							CheckException(false, Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Non-texture value supplied to texture uniform!")
						}

						//Upload slot ID to shader
						glUniform1i(uniformLocation, imageSlotCounter);

						//Increment slot counter
						imageSlotCounter++;
						break;
					case SpvType::UInt:
						switch(dims) {
							case 1:
								glUniform1ui(uniformLocation, std::any_cast<unsigned int>(item.data));
								break;
							case 2:
								glUniform2uiv(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::uvec2>(item.data)));
								break;
							case 3:
								glUniform3uiv(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::uvec3>(item.data)));
								break;
							case 4:
								glUniform4uiv(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::uvec4>(item.data)));
								break;
						}
						break;
					case SpvType::Float:
						switch(dims) {
							case 1:
								glUniform1f(uniformLocation, std::any_cast<float>(item.data));
								break;
							case 2:
								glUniform2fv(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::vec2>(item.data)));
								break;
							case 3:
								glUniform3fv(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::vec3>(item.data)));
								break;
							case 4:
								glUniform4fv(uniformLocation, 1, glm::value_ptr(std::any_cast<glm::vec4>(item.data)));
								break;
							case 6:
								glUniformMatrix2fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::mat2>(item.data)));
								break;
							case 7:
								glUniformMatrix2x3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::mat2x3>(item.data)));
								break;
							case 8:
								glUniformMatrix2x4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::mat2x4>(item.data)));
								break;
							case 10:
								glUniformMatrix3x2fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::mat3x2>(item.data)));
								break;
							case 11:
								glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::mat3>(item.data)));
								break;
							case 12:
								glUniformMatrix3x4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::mat3x4>(item.data)));
								break;
							case 14:
								glUniformMatrix4x2fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::mat4x2>(item.data)));
								break;
							case 15:
								glUniformMatrix4x3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::mat4x3>(item.data)));
								break;
							case 16:
								glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(std::any_cast<glm::mat4>(item.data)));
								break;
						}
						break;
					case SpvType::Double:
					case SpvType::Int64:
					case SpvType::UInt64:
						Handle64BitTypes(uniformLocation, item, info, dims);
				}
			} catch(const std::bad_cast&) {
				CheckException(false, Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Failed cast of shader upload value to type specified in target!")
			}

			//Restore previous shader (only if we weren't bound before)
			if(currentlyBound != -1) {
				Unbind();
				glUseProgram(currentlyBound);
			}
		}
	}

	void Shader::UploadCacaoData(glm::mat4 projection, glm::mat4 view, glm::mat4 transform) {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetThreadID()) {
			//Invoke OpenGL (ES) on the main thread
			InvokeGL([this, projection, view, transform]() {
				this->UploadCacaoData(projection, view, transform);
			});
			return;
		}
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot upload Cacao Engine data to uncompiled shader!")

		//Bind UBO
		glBindBuffer(GL_UNIFORM_BUFFER, nativeData->cacaoDataUBO);

		//Upload data
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(transform));

		//Unbind UBO
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}