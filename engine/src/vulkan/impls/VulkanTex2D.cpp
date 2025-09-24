#include "VulkanTex2D.hpp"
#include "Cacao/Engine.hpp"
#include "VulkanModule.hpp"

namespace Cacao {
	void VulkanTex2DImpl::Realize(bool& success) {
		//TODO
	}

	void VulkanTex2DImpl::DropRealized() {
		//TODO
	}

	Tex2D::Impl* VulkanModule::ConfigureTex2D() {
		return new VulkanTex2DImpl();
	}
}