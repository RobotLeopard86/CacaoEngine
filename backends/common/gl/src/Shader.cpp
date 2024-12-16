#include "Graphics/Shader.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "GLShaderData.hpp"
#include "Core/Assert.hpp"
#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "GLUtils.hpp"
#include "Utilities/AssetManager.hpp"
#include "Graphics/Material.hpp"

#include "glad/gl.h"
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

//See GLShaderData.hpp for why there's this "impl" thing
//Basically this just makes it easier to use and keeps the arrow operator method of
//accessing native data intact
#define nd (&(this->nativeData->impl))

namespace Cacao {
	//Required static member initialization
	GLuint ShaderDataImpl::uboIndexCounter = 1;

	//Sets up shader data according to the shader spec
	void PrepShaderData(const ShaderSpec& spec, ShaderDataImpl* mod, std::vector<uint32_t> vertSpv, std::vector<uint32_t> fragSpv) {
		CheckException(mod, Exception::GetExceptionCodeFromMeaning("NullValue"), "Passed-in shader data pointer is invalid!");

		//Convert SPIR-V to GLSL

		//Create common options
		spirv_cross::CompilerGLSL::Options options;
		options.es = false;
		options.version = 410;
		options.enable_420pack_extension = false;

		//Parse SPIR-V IR
		spirv_cross::Parser vertParse(std::move(vertSpv));
		vertParse.parse();
		spirv_cross::Parser fragParse(std::move(fragSpv));
		fragParse.parse();

		//Load vertex shader
		spirv_cross::ParsedIR& vir = vertParse.get_parsed_ir();
		bool vhlsl = vir.source.hlsl;
		spirv_cross::CompilerGLSL vertGLSL(std::move(vir));
		spirv_cross::ShaderResources vertRes = vertGLSL.get_shader_resources();
		vertGLSL.set_common_options(options);

		//Fix HLSL names because SPIRV-Cross likes to mess them up
		if(vhlsl) {
			for(auto& ubo : vertRes.uniform_buffers) {
				if(ubo.name.compare("type.cacao_globals") == 0 || ubo.name.compare("cacao_globals") == 0 || ubo.name.compare("type.ConstantBuffer.CacaoGlobals") == 0 || ubo.name.compare("globals") == 0) {
					vertGLSL.set_name(ubo.base_type_id, "CacaoGlobals");
				}
				if(ubo.name.compare("type.object_data") == 0 || ubo.name.compare("object_data") == 0 || ubo.name.compare("type.ConstantBuffer.ObjectData") == 0 || ubo.name.compare("object") == 0) {
					vertGLSL.set_name(ubo.base_type_id, "ObjectData");
				}
			}
			for(auto& out : vertRes.stage_outputs) {
				if(out.name.starts_with("out.var.")) {
					std::stringstream newName;
					newName << "V2F." << out.name.substr(8, out.name.size());
					vertGLSL.set_name(out.id, newName.str());
				}
			}
		}
		for(auto& pcb : vertRes.push_constant_buffers) {
			std::string typeName = vertGLSL.get_name(pcb.base_type_id);
			if(typeName.compare("type.transform") == 0 || typeName.compare("transform") == 0 || typeName.compare("type.PushConstant.Transformation") == 0 || typeName.compare("Transformation") == 0) {
				vertGLSL.set_name(pcb.base_type_id, "Transformation");
				vertGLSL.set_name(pcb.id, "transform");
			}
		}

		//Load fragment shader
		spirv_cross::ParsedIR& fir = fragParse.get_parsed_ir();
		bool fhlsl = fir.source.hlsl;
		spirv_cross::CompilerGLSL fragGLSL(std::move(fragParse.get_parsed_ir()));
		fragGLSL.set_common_options(options);
		spirv_cross::ShaderResources fragRes = fragGLSL.get_shader_resources();

		//Fix HLSL names because SPIRV-Cross likes to mess them up
		if(fhlsl) {
			for(auto& in : fragRes.stage_inputs) {
				if(in.name.starts_with("in.var.")) {
					std::stringstream newName;
					newName << "V2F." << in.name.substr(7, in.name.size());
					fragGLSL.set_name(in.id, newName.str());
				}
			}
		}

		//Remove image descriptor decorations
		for(auto& img : fragRes.sampled_images) {
			fragGLSL.unset_decoration(img.id, spv::DecorationDescriptorSet);
		}

		//Emit GLSL
		mod->vertexCode = vertGLSL.compile();
		mod->fragmentCode = fragGLSL.compile();

		//Get shader data block size and offsets
		bool foundSD = false;
		for(auto ubo : vertRes.uniform_buffers) {
			if(ubo.name.compare("ObjectData") == 0) {
				spirv_cross::SPIRType type = vertGLSL.get_type(ubo.base_type_id);
				mod->shaderDataSize = vertGLSL.get_declared_struct_size(type);
				for(unsigned int i = 0; i < type.member_types.size(); ++i) {
					mod->offsets.insert_or_assign(vertGLSL.get_member_name(ubo.base_type_id, i), vertGLSL.type_struct_member_offset(type, i));
				}
				foundSD = true;
				break;
			}
		}
		if(!foundSD) {
			mod->shaderDataSize = 0;
		}

		//Gather list of textures
		std::vector<std::string> texNames;
		for(auto image : fragRes.sampled_images) {
			texNames.push_back(image.name);
		}

		//Make sure that shader data matches spec
		for(const ShaderItemInfo& sii : spec) {
			CheckException((sii.type == SpvType::SampledImage && std::find(texNames.cbegin(), texNames.cend(), sii.name) != texNames.cend()) || mod->offsets.contains(sii.name), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Value found in shader spec that is not present in shader!");
		}
		for(auto offset : mod->offsets) {
			CheckException(std::find_if(spec.begin(), spec.end(), [&offset](const ShaderItemInfo& sii) { return offset.first.compare(sii.name) == 0; }) != spec.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Value found in shader that is not in shader spec!");
		}
	}

	Shader::Shader(std::string vertexPath, std::string fragmentPath, ShaderSpec spec)
	  : Asset(false), bound(false), specification(spec) {
		//Validate that these paths exist
		CheckException(std::filesystem::exists(vertexPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create a shader from a non-existent vertex shader file!");
		CheckException(std::filesystem::exists(fragmentPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create a shader from a non-existent fragment shader file!");

		//Load SPIR-V code

		//Open file streams
		FILE* vf = fopen(vertexPath.c_str(), "rb");
		CheckException(vf, Exception::GetExceptionCodeFromMeaning("FileOpenFailure"), "Failed to open vertex shader file!");
		FILE* ff = fopen(fragmentPath.c_str(), "rb");
		CheckException(ff, Exception::GetExceptionCodeFromMeaning("FileOpenFailure"), "Failed to open fragment shader file!");

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
		if(fread(vbuf.data(), sizeof(uint32_t), vlen, vf) != std::size_t(vlen)) {
			//Clean up file streams before exception throw
			fclose(vf);
			fclose(ff);
			CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to read vertex shader data from file!");
		}
		if(fread(fbuf.data(), sizeof(uint32_t), flen, ff) != std::size_t(flen)) {
			//Clean up file streams before exception throw
			fclose(vf);
			fclose(ff);
			CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to read fragment shader data from file!");
		}

		//Close file streams
		fclose(vf);
		fclose(ff);

		//Create native data
		nativeData.reset(new ShaderData());

		//Pre-process shader data
		PrepShaderData(specification, nd, vbuf, fbuf);

		//Apply unused transform flag
		if(currentShaderUnusedTransformFlag) {
			nd->unusedTransform = true;
			currentShaderUnusedTransformFlag = false;
		}

		//Check for images in spec
		nd->hasImages = std::find_if(specification.cbegin(), specification.cend(), [](const ShaderItemInfo& sii) {
			return sii.type == SpvType::SampledImage;
		}) != specification.cend();
	}

	Shader::Shader(std::vector<uint32_t>& vertex, std::vector<uint32_t>& fragment, ShaderSpec spec)
	  : Asset(false), bound(false), specification(spec) {
		//Create native data
		nativeData.reset(new ShaderData());

		//Pre-process shader data
		PrepShaderData(specification, nd, vertex, fragment);

		//Apply unused transform flag
		if(currentShaderUnusedTransformFlag) {
			nd->unusedTransform = true;
			currentShaderUnusedTransformFlag = false;
		}
	}

	void Shader::CompileSync() {
		CompileAsync().get();
	}

	std::shared_future<void> Shader::CompileAsync() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			//Invoke OpenGL on the engine thread
			return InvokeGL([this]() {
				this->CompileAsync();
			});
		}
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled shader!");

		//Create vertex shader base
		GLuint compiledVertexShader = glCreateShader(GL_VERTEX_SHADER);
		const GLchar* vertexSrc = nd->vertexCode.c_str();

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
			CheckException(false, Exception::GetExceptionCodeFromMeaning("GLError"), std::string("Vertex shader compilation failure: ") + infoLog.data());
			return {};
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
			CheckException(false, Exception::GetExceptionCodeFromMeaning("GLError"), std::string("Fragment shader compilation failure: ") + infoLog.data());
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
			CheckException(false, Exception::GetExceptionCodeFromMeaning("GLError"), std::string("Shader linking failure: ") + infoLog.data());
			return {};
		}

		//Detach and delete linked shaders
		glDetachShader(program, compiledFragmentShader);
		glDetachShader(program, compiledVertexShader);
		glDeleteShader(compiledVertexShader);
		glDeleteShader(compiledFragmentShader);

		//Link globals UBO
		GLuint globalUBOIdx = glGetUniformBlockIndex(program, "CacaoGlobals");
		CheckException(globalUBOIdx != GL_INVALID_INDEX, Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "Shader does not contain the Cacao Engine globals uniform block!");
		glUniformBlockBinding(program, globalUBOIdx, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalsUBO);

		//Set up transformation matrix
		if(!nd->unusedTransform) {
			GLint transformULoc = glGetUniformLocation(program, "transform.transform");
			CheckException(transformULoc != -1, Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "Shader does not contain the transformation matrix uniform!");
			nd->transformLoc = transformULoc;
		}

		//Confirm object data UBO
		GLuint objectUBOIdx = glGetUniformBlockIndex(program, "ObjectData");
		if(nd->offsets.size() > 0) {
			CheckException(objectUBOIdx != GL_INVALID_INDEX, Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "Shader contains non-texture uniforms but has no object data uniform block!");

			//Link object data UBO binding
			nd->shaderUBOBinding = ShaderDataImpl::uboIndexCounter;
			glUniformBlockBinding(program, objectUBOIdx, nd->shaderUBOBinding);

			//Increment UBO index counter
			ShaderDataImpl::uboIndexCounter++;
		}

		//Set GPU ID and compiled values
		nd->gpuID = program;
		compiled = true;

		//Return an empty future
		return {};
	}

	void Shader::Release() {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			//Try to invoke OpenGL and throw any exceptions back to the initial caller
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

		glDeleteProgram(nd->gpuID);
		compiled = false;
	}

	void Shader::Bind() {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetMainThreadID(), Exception::GetExceptionCodeFromMeaning("BadThread"), "Cannot bind shader in non-rendering thread!");
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled shader!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound shader!");

		glUseProgram(nd->gpuID);
		activeShader = nd;
		bound = true;
	}

	void Shader::Unbind() {
		CheckException(std::this_thread::get_id() == Engine::GetInstance()->GetMainThreadID(), Exception::GetExceptionCodeFromMeaning("BadThread"), "Cannot unbind shader in non-rendering thread!");
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot unbind uncompiled shader!");
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound shader!");

		//Clear current program
		glUseProgram(0);
		activeShader = nullptr;

		bound = false;
	}

	void Shader::UploadCacaoGlobals(glm::mat4 projection, glm::mat4 view) {
		if(std::this_thread::get_id() != Engine::GetInstance()->GetMainThreadID()) {
			//Invoke OpenGL on the engine thread
			InvokeGL([projection, view]() {
				UploadCacaoGlobals(projection, view);
			});
			return;
		}

		//Bind UBO
		glBindBuffer(GL_UNIFORM_BUFFER, globalsUBO);

		//Upload data
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

		//Unbind UBO
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void Shader::_BackendDestruct() {}

	std::shared_ptr<Material> Shader::CreateMaterial() {
		AssetHandle<Shader> selfHandle = AssetManager::GetInstance()->GetHandleFromPointer(this);

		//Unfortunately, we have to do it this way because make_shared doesn't work well with friend classes
		Material* m;
		if(selfHandle.IsNull()) {
			//We should never have to do this except for engine-internal shaders that aren't cached
			m = new Material(this);
		} else {
			m = new Material(selfHandle);
		}
		return std::shared_ptr<Material>(m);
	}
}