#include "Graphics/Material.hpp"

#include "VulkanCoreObjects.hpp"
#include "VkUtils.hpp"
#include "VkShader.hpp"
#include "ActiveItems.hpp"

#define shaderND (&(shader->nativeData->impl))

namespace Cacao {
	struct Material::MaterialData {
		Allocated<vk::Buffer> ubo;
		void* uboMem;
	};

	void Material::_CommonInit() {
		//Create native data
		nativeData.reset(new MaterialData());

		//Yeah... we're not active when constructed, right?
		active = false;

		//We don't need to do anything else if we don't need a shader data buffer
		if(shaderND->shaderDataSize <= 0) return;

		//Create a uniform buffer
		vk::BufferCreateInfo uboCI({}, shaderND->shaderDataSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo allocCI({}, vma::MemoryUsage::eCpuToGpu);
		try {
			auto [buf, alloc] = allocator.createBuffer(uboCI, allocCI);
			nativeData->ubo = {.alloc = alloc, .obj = buf};
		} catch(vk::SystemError& err) {
			std::stringstream emsg;
			emsg << "Failed to create material uniform buffer: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str());
		}

		//Map UBO memory
		CheckException(allocator.mapMemory(nativeData->ubo.alloc, &(nativeData->uboMem)) == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("Vulkan"), "Failed to map material uniform buffer!");
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
		if(shaderND->shaderDataSize <= 0) return;
		allocator.unmapMemory(nativeData->ubo.alloc);
		allocator.destroyBuffer(nativeData->ubo.obj, nativeData->ubo.alloc);
	}

	void Material::Activate() {
		CheckException(!active, Exception::GetExceptionCodeFromMeaning("BadState"), "Cannot activate active material!");
		CheckException(shader->IsCompiled(), Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot activate material with uncompiled shader!");
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot activate material when their is no active frame!");

		//Bind shader
		shader->Bind();

		//Check if we have to do anything else (some shaders have no parameters)
		if(shaderND->shaderDataSize == 0 && shaderND->imageSlots.size() == 0) goto activate_done;

		//Upload everything to the GPU that isn't an image
		if(shaderND->shaderDataSize != 0) {
			std::size_t currentsz;
			void* current;
			for(auto kv : values) {
				//Don't bother if it's not an image
				if(kv.second.index() > 14) continue;

				//Get buffer offset
				uint32_t offset = shaderND->offsets.at(kv.first);

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

				//Do the copy
				std::memcpy((unsigned char*)nativeData->uboMem + offset, current, currentsz);

				//Free the memory used for the copy source
				free(current);
			}
		}

		//Bind UBO
		if(shaderND->shaderDataSize > 0) {
			vk::DescriptorBufferInfo dbi(nativeData->ubo.obj, 0, vk::WholeSize);
			vk::WriteDescriptorSet wds(VK_NULL_HANDLE, 0, 0, 1, vk::DescriptorType::eUniformBuffer, VK_NULL_HANDLE, &dbi);
			activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);
		}

		//Bind any and all textures
		for(auto& kv : values) {
			ValueContainer& vc = kv.second;
			if(vc.index() < 15) continue;

			//Get the image slot
			int slot = shaderND->imageSlots.at(kv.first).binding;

			//Actually bind the texture
			switch(vc.index()) {
				case 15: {
					RawVkTexture* rvkt = static_cast<RawVkTexture*>(std::get<RawTexture*>(vc));
					*(rvkt->slot) = slot;
					vk::DescriptorImageInfo dii(VK_NULL_HANDLE, rvkt->view, vk::ImageLayout::eShaderReadOnlyOptimal);
					vk::WriteDescriptorSet wds(VK_NULL_HANDLE, slot, 0, vk::DescriptorType::eCombinedImageSampler, dii);
					activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);
					break;
				}
				case 16:
					std::get<Texture*>(vc)->Bind(slot);
					break;
				case 17:
					std::get<AssetHandle<Cubemap>>(vc)->Bind(slot);
					break;
				case 18:
					std::get<AssetHandle<Texture2D>>(vc)->Bind(slot);
					break;
				case 19:
					std::get<std::shared_ptr<UIView>>(vc)->Bind(slot);
					break;
			}
		}

	activate_done:
		active = true;
	}

	void Material::Deactivate() {
		CheckException(active, Exception::GetExceptionCodeFromMeaning("BadState"), "Cannot deactivate inactive material!");
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot deactivate material when their is no active frame!");

		//Unbind UBO
		if(shaderND->shaderDataSize > 0) {
			vk::DescriptorBufferInfo dbi(VK_NULL_HANDLE, 0, vk::WholeSize);
			vk::WriteDescriptorSet wds(VK_NULL_HANDLE, 0, 0, 1, vk::DescriptorType::eUniformBuffer, VK_NULL_HANDLE, &dbi);
			activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);
		}

		//Unbind all textures
		for(auto& kv : values) {
			ValueContainer& vc = kv.second;
			if(vc.index() < 15) continue;
			switch(vc.index()) {
				case 15: {
					vk::DescriptorImageInfo dii(VK_NULL_HANDLE, nullView, vk::ImageLayout::eShaderReadOnlyOptimal);
					vk::WriteDescriptorSet wds(VK_NULL_HANDLE, *(static_cast<RawVkTexture*>(std::get<RawTexture*>(vc))->slot), 0, vk::DescriptorType::eCombinedImageSampler, dii);
					activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);
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