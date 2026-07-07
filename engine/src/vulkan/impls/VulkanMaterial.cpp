#include "VulkanMaterial.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "VulkanModule.hpp"
#include "CommandBufferCast.hpp"

namespace Cacao {
	void VulkanMaterialImpl::Bake(bool& success) {
		success = true;
	}

	void VulkanMaterialImpl::Discard() {}

	Material::Impl* VulkanModule::ConfigureMaterial() {
		return new VulkanMaterialImpl();
	}
}