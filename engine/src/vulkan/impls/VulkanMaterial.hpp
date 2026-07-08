#pragma once

#include "impl/Material.hpp"

#include "VulkanModule.hpp"

namespace Cacao {
	class VulkanMaterialImpl : public Material::Impl {
	  public:
		void Upload(std::unique_ptr<CommandBuffer>& cmd) override;
	};
}