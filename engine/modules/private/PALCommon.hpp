#pragma once

#include <string>
#include <memory>
#include <map>

#include "Cacao/DllHelper.hpp"

namespace Cacao {
	class CACAO_API PALModule {
	  public:
		const std::string id;

		virtual void Init() = 0;
		virtual void Term() = 0;
		virtual void Connect() = 0;
		virtual void Disconnect() = 0;
		virtual void Destroy() = 0;

		/* ------------------------------------------- *\
		|*      PLACEHOLDER: IMPL CONFIGURATORS        *|
		\* ------------------------------------------- */

		virtual ~PALModule() {}

		bool Initialized() {
			return didInit;
		}
		bool Connected() {
			return connected;
		}

	  protected:
		PALModule(const std::string& id)
		  : id(id), didInit(false), connected(false) {}

		bool didInit;
		bool connected;
	};
}

#define PAL_BACKED_IMPL(c)                             \
	struct Cacao::c::Impl {                            \
		std::shared_ptr<Cacao::PAL##c##Interface> pal; \
	};
