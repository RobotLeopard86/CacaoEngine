#include "Cacao/PAL.hpp"
#include "PALCommon.hpp"
#include "PALImpl.hpp"

namespace Cacao {
	PAL::PAL() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
		impl->modInit = false;
	}

	PAL::~PAL() {
		impl->mod.reset();
		impl->modInit = false;
	}

	//Stubs

	void PAL::SetModule(const std::string& mod) {}

	bool PAL::TryInitActiveModule() {
		return true;
	}

	void PAL::Unload() {}
}
