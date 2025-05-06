#pragma once

#include <memory>

#include "Cacao/DllHelper.hpp"
#include "PALModule.hpp"

#define PAL_BACKED_IMPL(c)                             \
	struct Cacao::c::Impl {                            \
		std::shared_ptr<Cacao::PAL##c##Interface> pal; \
	};

namespace Cacao {
	class CACAO_API PALInterface {
	  public:
		virtual ~PALInterface() {}

	  protected:
		PALInterface() {}

		//We use the module for a reference count so it can't be unloaded while objects are alive
		std::shared_ptr<PALModule> m;
	};
}