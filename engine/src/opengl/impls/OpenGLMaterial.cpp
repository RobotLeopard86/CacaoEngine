#include "OpenGLMaterial.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "OpenGLModule.hpp"
#include "CommandBufferCast.hpp"

namespace Cacao {
	void OpenGLMaterialImpl::Upload(std::unique_ptr<CommandBuffer>& cmd) {}

	Material::Impl* OpenGLModule::ConfigureMaterial() {
		return new OpenGLMaterialImpl();
	}
}