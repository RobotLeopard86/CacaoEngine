#include "VulkanMesh.hpp"
#include "Cacao/Engine.hpp"
#include "VulkanModule.hpp"

namespace Cacao {
	void VulkanMeshImpl::Realize(bool& success) {
		//TODO
	}

	void VulkanMeshImpl::DropRealized() {
		//TODO
	}

	Mesh::Impl* VulkanModule::ConfigureMesh() {
		return new VulkanMeshImpl();
	}
}