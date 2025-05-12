#pragma once

#include "PALCommon.hpp"

namespace Cacao {
	class VulkanModule : public PALModule {
	  public:
		void Init() override;
		void Term() override;
		void Connect() override;
		void Disconnect() override;

		/* ------------------------------------------- *\
		|*      PLACEHOLDER: IMPL CONFIGURATORS        *|
		\* ------------------------------------------- */

		VulkanModule()
		  : PALModule("vulkan") {}
	};
}