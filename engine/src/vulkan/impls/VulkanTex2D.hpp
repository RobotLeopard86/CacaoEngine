#pragma once

#include "impl/Tex2D.hpp"

#include "VulkanModule.hpp"

namespace Cacao {
	class VulkanTex2DImpl : public Tex2D::Impl {
	  public:
		void Bake(bool& success) override;
		void Discard() override;

		//Image memory and view
		ViewImage vi;

		//Image format
		vk::Format format;

		//Sampler
		vk::Sampler sampler;
	};
}