#include "Module.hpp"
#include "ModuleCreators.hpp"
#ifdef __linux__
#include "LinuxRouter.hpp"
#endif

namespace Cacao {
	std::shared_ptr<PALModule> CreateOpenGLModule() {
		gl = std::make_shared<OpenGLModule>();
		return std::static_pointer_cast<PALModule>(gl);
	}

    void OpenGLModule::Init() {}
    void OpenGLModule::Term() {}
    void OpenGLModule::Connect() {}
    void OpenGLModule::Disconnect() {}
    void OpenGLModule::Destroy() {}
    void OpenGLModule::SetVSync(bool state) {}
}