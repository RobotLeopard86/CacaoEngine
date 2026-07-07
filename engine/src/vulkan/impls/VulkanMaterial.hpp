#pragma once

#include "impl/Material.hpp"

#include "VulkanModule.hpp"

namespace Cacao {
	class VulkanMaterialImpl : public Material::Impl {
	  public:
		void Bake(bool& success) override;
		void Discard() override;
	};
}