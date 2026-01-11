#pragma once

#include "impl/Mesh.hpp"

#include "VulkanModule.hpp"

namespace Cacao {
	class VulkanMeshImpl : public Mesh::Impl {
	  public:
		void Realize(bool& success) override;
		void DropRealized() override;

		//Vertex Buffer and Index Buffer
		Allocated<vk::Buffer> vbo, ibo;
	};
}