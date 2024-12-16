#include "Graphics/Material.hpp"

#include "GLUtils.hpp"
#include "GLShaderData.hpp"

#define shaderND (&(shader->nativeData->impl))

namespace Cacao {
	struct Material::MaterialData {
		GLuint ubo;
		bool didAllocUBO = false;
	};

	void Material::_CommonInit() {
		//Create native data
		nativeData.reset(new MaterialData());

		//Yeah... we're not active when constructed, right?
		active = false;

		//We don't need to do anything else if we don't need a shader data buffer (it's UBO bi)
		if(shaderND->offsets.size() <= 0) return;

		//Yes, we allocated a UBO
		nativeData->didAllocUBO = true;

		//Create a uniform buffer
		glGenBuffers(1, &nativeData->ubo);
	}

	Material::Material(AssetHandle<Shader> shader)
	  : shader({.shaderHandle = shader, .rawShader = shader.GetManagedAsset().get()}) {
		_CommonInit();
	}

	Material::Material(Shader* rawShader)
	  : shader({.shaderHandle = {}, .rawShader = rawShader}) {
		_CommonInit();
	}

	Material::Material(const Material& other) {
		shader = other.shader;
		values = other.values;
		_CommonInit();
	}

	Material& Material::operator=(const Material& other) {
		if(this == &other) return *this;

		//Copy other to become the new this
		Material newObj(other);
		*this = newObj;

		return *this;
	}

	Material::~Material() {
		if(active) Deactivate();
		if(!nativeData->didAllocUBO) return;
		glDeleteBuffers(1, &nativeData->ubo);
	}

	void Material::Activate() {
		CheckException(!active, Exception::GetExceptionCodeFromMeaning("BadState"), "Cannot activate active material!");
		CheckException(shader->IsCompiled(), Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot activate material with uncompiled shader!");

		//Bind shader
		shader->Bind();

		//Check if we have to do anything else (some shaders have no parameters)
		if(!nativeData->didAllocUBO && !shaderND->hasImages) goto activate_done;

		//Upload everything to the GPU that isn't an image
		if(nativeData->didAllocUBO) {
			std::size_t currentsz;
			void* current;
			for(auto kv : values) {
				//Don't bother if it's not an image
				if(kv.second.index() > 14) continue;

				//Extract the value to a pointer for copying
				switch(kv.second.index()) {
					case 0: {
						currentsz = sizeof(int);
						int v = std::get<int>(kv.second);
						current = malloc(currentsz);
						*(int*)current = v;
						break;
					}
					case 1: {
						currentsz = sizeof(unsigned int);
						unsigned int v = std::get<unsigned int>(kv.second);
						current = malloc(currentsz);
						*(unsigned int*)current = v;
						break;
					}
					case 2: {
						currentsz = sizeof(float);
						float v = std::get<float>(kv.second);
						current = malloc(currentsz);
						*(float*)current = v;
						break;
					}
					case 3: {
						currentsz = sizeof(glm::vec2);
						glm::vec2 v = std::get<glm::vec2>(kv.second);
						current = malloc(currentsz);
						*(glm::vec2*)current = v;
						break;
					}
					case 4: {
						currentsz = sizeof(glm::vec3);
						glm::vec3 v = std::get<glm::vec3>(kv.second);
						current = malloc(currentsz);
						*(glm::vec3*)current = v;
						break;
					}
					case 5: {
						currentsz = sizeof(glm::vec4);
						glm::vec4 v = std::get<glm::vec4>(kv.second);
						current = malloc(currentsz);
						*(glm::vec4*)current = v;
						break;
					}
					case 6: {
						currentsz = sizeof(glm::mat2);
						glm::mat2 v = std::get<glm::mat2>(kv.second);
						current = malloc(currentsz);
						*(glm::mat2*)current = v;
						break;
					}
					case 7: {
						currentsz = sizeof(glm::mat2x3);
						glm::mat2x3 v = std::get<glm::mat2x3>(kv.second);
						current = malloc(currentsz);
						*(glm::mat2x3*)current = v;
						break;
					}
					case 8: {
						currentsz = sizeof(glm::mat2x4);
						glm::mat2x4 v = std::get<glm::mat2x4>(kv.second);
						current = malloc(currentsz);
						*(glm::mat2x4*)current = v;
						break;
					}
					case 9: {
						currentsz = sizeof(glm::mat3x2);
						glm::mat3x2 v = std::get<glm::mat3x2>(kv.second);
						current = malloc(currentsz);
						*(glm::mat3x2*)current = v;
						break;
					}
					case 10: {
						currentsz = sizeof(glm::mat3);
						glm::mat3 v = std::get<glm::mat3>(kv.second);
						current = malloc(currentsz);
						*(glm::mat3*)current = v;
						break;
					}
					case 11: {
						currentsz = sizeof(glm::mat3x4);
						glm::mat3x4 v = std::get<glm::mat3x4>(kv.second);
						current = malloc(currentsz);
						*(glm::mat3x4*)current = v;
						break;
					}
					case 12: {
						currentsz = sizeof(glm::mat4x2);
						glm::mat4x2 v = std::get<glm::mat4x2>(kv.second);
						current = malloc(currentsz);
						*(glm::mat4x2*)current = v;
						break;
					}
					case 13: {
						currentsz = sizeof(glm::mat4x3);
						glm::mat4x3 v = std::get<glm::mat4x3>(kv.second);
						current = malloc(currentsz);
						*(glm::mat4x3*)current = v;
						break;
					}
					case 14: {
						currentsz = sizeof(glm::mat4);
						glm::mat4 v = std::get<glm::mat4>(kv.second);
						current = malloc(currentsz);
						*(glm::mat4*)current = v;
						break;
					}
					default:
						break;
				}

				//Bind buffer for copying (get the old one to restore)
				GLint prevBound;
				glGetIntegerv(GL_UNIFORM_BUFFER, &prevBound);
				glBindBuffer(GL_UNIFORM_BUFFER, nativeData->ubo);

				//Do the copy
				glBufferSubData(GL_UNIFORM_BUFFER, shaderND->offsets.at(kv.first), currentsz, current);

				//Free the memory used for the copy source
				free(current);

				//Restore old buffer
				if(prevBound != 0) glBindBuffer(GL_UNIFORM_BUFFER, prevBound);
			}

			//Bind UBO to shader
			glBindBufferBase(GL_UNIFORM_BUFFER, shaderND->shaderUBOBinding, nativeData->ubo);
		}

		{
			int imageSlotCounter = 0;

			//Bind any and all textures
			for(auto& kv : values) {
				ValueContainer& vc = kv.second;
				if(vc.index() < 15) continue;

				//Actually bind the texture
				switch(vc.index()) {
					case 15: {
						RawGLTexture* rglt = static_cast<RawGLTexture*>(std::get<RawTexture*>(vc));
						*(rglt->slot) = imageSlotCounter;
						glActiveTexture(GL_TEXTURE0 + imageSlotCounter);
						glBindTexture(GL_TEXTURE_2D, rglt->texObj);
						break;
					}
					case 16:
						std::get<Texture*>(vc)->Bind(imageSlotCounter);
						break;
					case 17:
						std::get<AssetHandle<Cubemap>>(vc)->Bind(imageSlotCounter);
						break;
					case 18:
						std::get<AssetHandle<Texture2D>>(vc)->Bind(imageSlotCounter);
						break;
					case 19:
						std::get<std::shared_ptr<UIView>>(vc)->Bind(imageSlotCounter);
						break;
				}

				//Upload slot ID to shader
				GLint uniformLocation = glGetUniformLocation(shaderND->gpuID, kv.first.c_str());
				CheckException(uniformLocation != -1, Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "Shader does not contain the requested texture uniform binding!");
				glUniform1i(uniformLocation, imageSlotCounter);

				//Increment the slot counter
				imageSlotCounter++;
			}
		}

	activate_done:
		active = true;
	}

	void Material::Deactivate() {
		CheckException(active, Exception::GetExceptionCodeFromMeaning("BadState"), "Cannot deactivate inactive material!");

		//Unbind UBO
		if(nativeData->didAllocUBO) {
			//Unbind UBO from shader
			glBindBufferBase(GL_UNIFORM_BUFFER, shaderND->shaderUBOBinding, 0);
		}

		//Unbind all textures
		for(auto& kv : values) {
			ValueContainer& vc = kv.second;
			if(vc.index() < 15) continue;
			switch(vc.index()) {
				case 15: {
					RawGLTexture* rglt = static_cast<RawGLTexture*>(std::get<RawTexture*>(vc));
					glActiveTexture(GL_TEXTURE0 + *(rglt->slot));
					glBindTexture(GL_TEXTURE_2D, 0);
					break;
				}
				case 16:
					std::get<Texture*>(vc)->Unbind();
					break;
				case 17:
					std::get<AssetHandle<Cubemap>>(vc)->Unbind();
					break;
				case 18:
					std::get<AssetHandle<Texture2D>>(vc)->Unbind();
					break;
				case 19:
					std::get<std::shared_ptr<UIView>>(vc)->Unbind();
					break;
			}
		}

		//Unbind shader
		shader->Unbind();

		active = false;
	}
}