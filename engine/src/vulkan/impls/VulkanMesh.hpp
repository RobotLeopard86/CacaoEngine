#pragma once

#include "impl/Mesh.hpp"

#include "Module.hpp"

namespace Cacao {
	class VulkanMeshImpl : public Mesh::Impl {
	  public:
		std::optional<std::shared_future<void>> Realize(bool& success) override;
		void DropRealized() override;
		bool DoWaitAsyncForSync() const override {
			return false;
		}

		//Vertex Buffer and Index Buffer
		Allocated<vk::Buffer> vbo, ibo;
	};
}