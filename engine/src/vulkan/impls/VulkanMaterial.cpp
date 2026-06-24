#include "VulkanMaterial.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "VulkanModule.hpp"
#include "CommandBufferCast.hpp"

namespace Cacao {
	void VulkanMaterialImpl::Realize(bool& success) {
		success = true;
	}

	void VulkanMaterialImpl::DropRealized() {}

	Material::Impl* VulkanModule::ConfigureMaterial() {
		return new VulkanMaterialImpl();
	}
}