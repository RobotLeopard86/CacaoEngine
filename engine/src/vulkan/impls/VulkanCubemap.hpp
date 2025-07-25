#pragma once

#include "impl/Cubemap.hpp"

#include "Module.hpp"

namespace Cacao {
	class VulkanCubemapImpl : public Cubemap::Impl {
	  public:
		std::optional<std::shared_future<void>> Realize() override;
		void DropRealized() override;
		bool DoWaitAsyncForSync() const override {
			return false;
		}

		//Image memory and view
		ViewImage vi;
	};
}