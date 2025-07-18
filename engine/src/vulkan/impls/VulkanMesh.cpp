#include "VulkanMesh.hpp"
#include "Cacao/Engine.hpp"
#include "Module.hpp"

namespace Cacao {
	std::optional<std::shared_future<void>> VulkanMeshImpl::Realize() {
		//TODO
		return std::nullopt;
	}

	void VulkanMeshImpl::DropRealized() {
		//TODO
	}

	Mesh::Impl* VulkanModule::ConfigureMesh() {
		return new VulkanMeshImpl();
	}
}