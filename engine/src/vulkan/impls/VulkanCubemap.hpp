#pragma once

#include "impl/Cubemap.hpp"

#include "VulkanModule.hpp"

namespace Cacao {
	class VulkanCubemapImpl : public Cubemap::Impl {
	  public:
		void Bake(bool& success) override;
		void Discard() override;

		//Image memory and view
		ViewImage vi;

		//Sampler
		vk::Sampler sampler;
	};
}