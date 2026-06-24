#pragma once

#include "impl/Material.hpp"

#include "VulkanModule.hpp"

namespace Cacao {
	class VulkanMaterialImpl : public Material::Impl {
	  public:
		void Realize(bool& success) override;
		void DropRealized() override;
	};
}