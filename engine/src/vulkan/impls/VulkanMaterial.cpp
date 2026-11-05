#include "VulkanMaterial.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "VulkanModule.hpp"
#include "VulkanShader.hpp"
#include "VulkanTex2D.hpp"
#include "VulkanCubemap.hpp"
#include "ImplAccessor.hpp"

#include "exathread.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "libcacaoasset.hpp"

#include <variant>
#include <ranges>

namespace Cacao {
	void VulkanMaterialImpl::Apply(CommandBuffer* cmd) {
		//Check that all values are set
		Check<BadValueException>(storage.size() == (shader->GetDescriptor().uniformParams.size() + shader->GetDescriptor().texParams.size()),
			"Material element storage is not the same size as descriptor parameters!");

		//Bake shader and textures if needed
		if(!shader->IsBaked()) shader->Bake();
		std::vector<exathread::Future<void>> bakers;
		for(auto& [name, val] : storage) {
			if(val.index() == std::variant_size_v<Material::ParamValue> - 2) {
				//Tex2D
				std::shared_ptr<Tex2D> tex = std::get<std::shared_ptr<Tex2D>>(val);
				if(!tex->IsBaked()) bakers.push_back(Engine::Get().GetThreadPool()->submit([](std::shared_ptr<Tex2D> tex) {
					tex->Bake();
				},
					tex));
			} else if(val.index() == std::variant_size_v<Material::ParamValue> - 1) {
				//Cubemap
				std::shared_ptr<Cubemap> cmap = std::get<std::shared_ptr<Cubemap>>(val);
				if(!cmap->IsBaked()) bakers.push_back(Engine::Get().GetThreadPool()->submit([](std::shared_ptr<Cubemap> cmap) {
					cmap->Bake();
				},
					cmap));
			}
		}
		std::optional<exathread::MultiFuture<void>> bakeFuts;
		if(bakers.size() > 0) bakeFuts.emplace(std::move(bakers));

		//While textures are baking, we can upload uniform parameters
		for(const libcacaoasset::Shader::Descriptor::UniformParameter& uparam : shader->GetDescriptor().uniformParams) {
			switch(uparam.type) {
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int: {
					uint8_t copySize = sizeof(int);
					int data = std::get<int>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, &data, copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt: {
					uint8_t copySize = sizeof(unsigned int);
					unsigned int data = std::get<unsigned int>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, &data, copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float: {
					uint8_t copySize = sizeof(float);
					float data = std::get<float>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, &data, copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Bool: {
					uint8_t copySize = sizeof(bool);
					bool data = std::get<bool>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, &data, copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int2: {
					uint8_t copySize = sizeof(glm::ivec2);
					glm::ivec2 data = std::get<glm::ivec2>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int3: {
					uint8_t copySize = sizeof(glm::ivec3);
					glm::ivec3 data = std::get<glm::ivec3>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int4: {
					uint8_t copySize = sizeof(glm::ivec4);
					glm::ivec4 data = std::get<glm::ivec4>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt2: {
					uint8_t copySize = sizeof(glm::uvec2);
					glm::uvec2 data = std::get<glm::uvec2>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt3: {
					uint8_t copySize = sizeof(glm::uvec3);
					glm::uvec3 data = std::get<glm::uvec3>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt4: {
					uint8_t copySize = sizeof(glm::uvec4);
					glm::uvec4 data = std::get<glm::uvec4>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float2: {
					uint8_t copySize = sizeof(glm::vec2);
					glm::vec2 data = std::get<glm::vec2>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float3: {
					uint8_t copySize = sizeof(glm::vec3);
					glm::vec3 data = std::get<glm::vec3>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float4: {
					uint8_t copySize = sizeof(glm::vec4);
					glm::vec4 data = std::get<glm::vec4>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float2x2: {
					uint8_t copySize = sizeof(glm::mat2);
					glm::mat2 data = std::get<glm::mat2>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float3x3: {
					uint8_t copySize = sizeof(glm::mat3);
					glm::mat3 data = std::get<glm::mat3>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float4x4: {
					uint8_t copySize = sizeof(glm::mat4);
					glm::mat4 data = std::get<glm::mat4>(storage[uparam.name]);
					std::memcpy(reinterpret_cast<unsigned char*>(RES_IMPL(Shader, Vulkan, *shader).materialData.mem) + uparam.bufferOffset, glm::value_ptr(data), copySize);
					break;
				}
				default: break;
			}
		}

		//Ensure textures are done baking
		if(bakeFuts.has_value()) bakeFuts->await();

		//Bind shader pipeline
		VulkanCommandBuffer* vcb = static_cast<VulkanCommandBuffer*>(cmd);
		VulkanShaderImpl& vkShader = RES_IMPL(Shader, Vulkan, *shader);
		vkShader.Bind(vcb, renderMode == libcacaoasset::Material::RenderMode::Transparent);

		//Bind textures to descriptors
		if(vkShader.descriptor.texParams.size() > 0) {
			std::vector<vk::DescriptorImageInfo> diis;
			auto writeView = shader->GetDescriptor().texParams | std::views::transform([this, &diis](const libcacaoasset::Shader::Descriptor::TextureParameter& tparam) {
				//Create image info
				vk::DescriptorImageInfo& dii = diis.emplace_back();
				if(!tparam.isCubemap) {
					VulkanTex2DImpl& tex = RES_IMPL(Tex2D, Vulkan, *std::get<std::shared_ptr<Tex2D>>(storage[tparam.name]));
					dii = vk::DescriptorImageInfo(tex.sampler, tex.vi.view, vk::ImageLayout::eShaderReadOnlyOptimal);
				} else {
					VulkanCubemapImpl& cmap = RES_IMPL(Cubemap, Vulkan, *std::get<std::shared_ptr<Cubemap>>(storage[tparam.name]));
					dii = vk::DescriptorImageInfo(cmap.sampler, cmap.vi.view, vk::ImageLayout::eShaderReadOnlyOptimal);
				}

				//Create descriptor write
				return vk::WriteDescriptorSet(VK_NULL_HANDLE, tparam.binding, 0, 1, vk::DescriptorType::eCombinedImageSampler, &dii);
			}) | std::views::common;
			std::vector<vk::WriteDescriptorSet> writes(writeView.begin(), writeView.end());
			vcb->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, vkShader.layout, 1, writes);
		}
	}

	Material::Impl* VulkanModule::ConfigureMaterial() {
		return new VulkanMaterialImpl();
	}
}