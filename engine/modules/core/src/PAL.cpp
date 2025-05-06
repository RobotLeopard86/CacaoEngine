#include "Cacao/PAL.hpp"
#include "PALCommon.hpp"
#include "PALImpl.hpp"

namespace Cacao {
	PAL::PAL() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	PAL::~PAL() {
		impl->gfx.reset();
		impl->win.reset();
	}
}
