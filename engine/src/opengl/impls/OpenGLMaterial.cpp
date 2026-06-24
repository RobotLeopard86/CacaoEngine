#include "OpenGLMaterial.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "OpenGLModule.hpp"
#include "CommandBufferCast.hpp"

namespace Cacao {
	void OpenGLMaterialImpl::Realize(bool& success) {
		success = true;
	}

	void OpenGLMaterialImpl::DropRealized() {}

	Material::Impl* OpenGLModule::ConfigureMaterial() {
		return new OpenGLMaterialImpl();
	}
}