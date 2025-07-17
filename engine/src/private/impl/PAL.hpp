#pragma once

#include <string>
#include <map>
#include <functional>

#include "Cacao/DllHelper.hpp"
#include "Cacao/PAL.hpp"

namespace Cacao {
	class CACAO_API PALModule {
	  public:
		const std::string id;

		virtual void Init() = 0;
		virtual void Term() = 0;
		virtual void Connect() = 0;
		virtual void Disconnect() = 0;
		virtual void Destroy() = 0;
		virtual void SetVSync(bool state) = 0;

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

	struct PAL::Impl {
		std::shared_ptr<PALModule> mod;
		std::map<std::string, std::function<std::shared_ptr<PALModule>()>> registry;
	};
}
