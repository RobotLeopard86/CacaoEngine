#include "OpenGLMaterial.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "OpenGLModule.hpp"
#include "CommandBufferCast.hpp"

namespace Cacao {
	void OpenGLMaterialImpl::Bake(bool& success) {
		success = true;
	}

	void OpenGLMaterialImpl::Discard() {}

	Material::Impl* OpenGLModule::ConfigureMaterial() {
		return new OpenGLMaterialImpl();
	}
}