#pragma once

#include "PALCommon.hpp"
#include "Cacao/PAL.hpp"

#include <memory>

namespace Cacao {
	struct PAL::Impl {
		std::shared_ptr<PALModule> win, gfx;
	};
}