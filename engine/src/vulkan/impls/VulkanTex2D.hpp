#pragma once

#include "impl/Tex2D.hpp"

#include "Module.hpp"

namespace Cacao {
	class VulkanTex2DImpl : public Tex2D::Impl {
	  public:
		std::optional<std::shared_future<void>> Realize(bool& success) override;
		void DropRealized() override;
		bool DoWaitAsyncForSync() const override {
			return false;
		}

		//Image memory and view
		ViewImage vi;

		//Image format
		vk::Format format;
	};
}