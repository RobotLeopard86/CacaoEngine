#include "Graphics/Material.hpp"

#include "VulkanCoreObjects.hpp"
#include "VkUtils.hpp"
#include "VkShader.hpp"

#define shaderND (&(shader->nativeData->impl))

namespace Cacao {
	struct Material::MaterialData {
		Allocated<vk::Buffer> ubo;
		void* uboMem;
	};

	Material::Material(AssetHandle<Shader> shader) {
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

		CheckException(allocator.mapMemory(nativeData->ubo.alloc, &(nativeData->uboMem)) == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("Vulkan"), "Failed to map material uniform buffer!");
	}

	void Material::Activate() {
	}

	void Material::Deactivate() {

		shader->Unbind();
	}
}