#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "PALCommon.hpp"
#include "ModuleCreators.hpp"

namespace Cacao {
	struct PAL::Impl {
		std::shared_ptr<PALModule> mod;
	};

	PAL::PAL() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	PAL::~PAL() {
		impl->mod.reset();
	}

	void PAL::SetModule(const std::string& mod) {
		Check<BadStateException>(impl->mod.use_count() <= 1, "Cannot change the module when objects referencing the old one still exist!");
		if(mod.compare("vulkan") == 0) {
			impl->mod = CreateVulkanModule();
		} else if(mod.compare("opengl") == 0) {
			//impl->mod = CreateOpenGLModule();
		}
	}

	bool PAL::InitializeModule() {
		return true;
	}

	void PAL::GfxConnect() {
	}

	void PAL::GfxDisconnect() {
	}

	void PAL::TerminateModule() {}
}
