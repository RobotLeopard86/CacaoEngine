#include "OpenGLMaterial.hpp"
#include "Cacao/Cubemap.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "OpenGLModule.hpp"
#include "OpenGLShader.hpp"
#include "OpenGLTex2D.hpp"
#include "OpenGLCubemap.hpp"
#include "ImplAccessor.hpp"

#include "glm/gtc/type_ptr.hpp"

namespace Cacao {
	void OpenGLMaterialImpl::Apply(CommandBuffer*) {
		//Check that all values are set
		Check<BadValueException>(storage.size() == (shader->GetDescriptor().uniformParams.size() + shader->GetDescriptor().texParams.size()),
			"Material element storage is not the same size as descriptor parameters!");

		//Bake shader if needed
		if(!shader->IsBaked()) shader->Bake();

		//Bind shader
		OpenGLShaderImpl& glShader = RES_IMPL(Shader, OpenGL, *shader);
		glShader.Bind(renderMode == libcacaoasset::Material::RenderMode::Transparent);

		//Bake and bind textures
		for(const libcacaoasset::Shader::Descriptor::TextureParameter& tparam : shader->GetDescriptor().texParams) {
			//Get texture object
			GLuint obj;
			if(tparam.isCubemap) {
				std::shared_ptr<Cubemap> cmap = std::get<std::shared_ptr<Cubemap>>(storage[tparam.name]);
				if(!cmap->IsBaked()) cmap->Bake();
				obj = RES_IMPL(Cubemap, OpenGL, *cmap).gpuTex;
			} else {
				std::shared_ptr<Tex2D> tex = std::get<std::shared_ptr<Tex2D>>(storage[tparam.name]);
				if(!tex->IsBaked()) tex->Bake();
				obj = RES_IMPL(Tex2D, OpenGL, *tex).gpuTex;
			}

			//Bind it
			glActiveTexture(GL_TEXTURE0 + tparam.binding);
			glBindTexture(tparam.isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, obj);

			//Inform shader about the texture binding
			std::string glslParamName = "material_" + tparam.name;
			GLint uniformLocation = glGetUniformLocation(glShader.program, glslParamName.c_str());
			Check<ExternalException>(uniformLocation != -1, "Failed to locate texture object for binding!");
			glUniform1i(uniformLocation, tparam.binding);
		}

		//Upload uniform parameters
		for(const libcacaoasset::Shader::Descriptor::UniformParameter& uparam : shader->GetDescriptor().uniformParams) {
			switch(uparam.type) {
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int: {
					uint8_t copySize = sizeof(int);
					int data = std::get<int>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, &data);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt: {
					uint8_t copySize = sizeof(unsigned int);
					unsigned int data = std::get<unsigned int>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, &data);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float: {
					uint8_t copySize = sizeof(float);
					float data = std::get<float>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, &data);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Bool: {
					uint8_t copySize = sizeof(bool);
					bool data = std::get<bool>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, &data);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int2: {
					uint8_t copySize = sizeof(glm::ivec2);
					glm::ivec2 data = std::get<glm::ivec2>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int3: {
					uint8_t copySize = sizeof(glm::ivec3);
					glm::ivec3 data = std::get<glm::ivec3>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int4: {
					uint8_t copySize = sizeof(glm::ivec4);
					glm::ivec4 data = std::get<glm::ivec4>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt2: {
					uint8_t copySize = sizeof(glm::uvec2);
					glm::uvec2 data = std::get<glm::uvec2>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt3: {
					uint8_t copySize = sizeof(glm::uvec3);
					glm::uvec3 data = std::get<glm::uvec3>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt4: {
					uint8_t copySize = sizeof(glm::uvec4);
					glm::uvec4 data = std::get<glm::uvec4>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float2: {
					uint8_t copySize = sizeof(glm::vec2);
					glm::vec2 data = std::get<glm::vec2>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float3: {
					uint8_t copySize = sizeof(glm::vec3);
					glm::vec3 data = std::get<glm::vec3>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float4: {
					uint8_t copySize = sizeof(glm::vec4);
					glm::vec4 data = std::get<glm::vec4>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float2x2: {
					uint8_t copySize = sizeof(glm::mat2);
					glm::mat2 data = std::get<glm::mat2>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float3x3: {
					uint8_t copySize = sizeof(glm::mat3);
					glm::mat3 data = std::get<glm::mat3>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float4x4: {
					uint8_t copySize = sizeof(glm::mat4);
					glm::mat4 data = std::get<glm::mat4>(storage[uparam.name]);
					glNamedBufferSubData(glShader.ubo, uparam.bufferOffset, copySize, glm::value_ptr(data));
					break;
				}
				default: break;
			}
		}
	}

	Material::Impl* OpenGLModule::ConfigureMaterial() {
		return new OpenGLMaterialImpl();
	}
}