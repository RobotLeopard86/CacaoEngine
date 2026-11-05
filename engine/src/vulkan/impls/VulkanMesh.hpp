#pragma once

#include "impl/Mesh.hpp"

#include "VulkanModule.hpp"

namespace Cacao {
	class VulkanMeshImpl : public Mesh::Impl {
	  public:
		void Bake(bool& success) override;
		void Discard() override;

		//Vertex Buffer and Index Buffer
		Allocated<vk::Buffer> vbo, ibo;
	};
}