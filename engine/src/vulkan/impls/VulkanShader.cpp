#include "VulkanShader.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "VulkanModule.hpp"
#include "CommandBufferCast.hpp"

namespace Cacao {
	void VulkanShaderImpl::Realize(bool& success) {
		success = true;
	}

	void VulkanShaderImpl::DropRealized() {}

	Shader::Impl* VulkanModule::ConfigureShader() {
		return new VulkanShaderImpl();
	}
}