#include "VulkanMesh.hpp"
#include "Cacao/Engine.hpp"
#include "VulkanModule.hpp"

namespace Cacao {
	std::optional<std::shared_future<void>> VulkanMeshImpl::Realize(bool& success) {
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