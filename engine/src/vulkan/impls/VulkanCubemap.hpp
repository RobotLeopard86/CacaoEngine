#pragma once

#include "impl/Cubemap.hpp"

#include "VulkanModule.hpp"

namespace Cacao {
	class VulkanCubemapImpl : public Cubemap::Impl {
	  public:
		void Realize(bool& success) override;
		void DropRealized() override;

		//Image memory and view
		ViewImage vi;
	};
}