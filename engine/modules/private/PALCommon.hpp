#pragma once

#include <functional>
#include <map>

#include "Cacao/DllHelper.hpp"

#include "dynalo/dynalo.hpp"

namespace Cacao {
	class PALInterface;

	class CACAO_API PALModule {
	  private:
		dynalo::library lib;

	  public:
		const std::string id;

		enum class FactoryType {
			Window,
			Shader,
			Tex2D,
			Cubemap,
			Mesh
		};

		const std::map<FactoryType, std::function<std::shared_ptr<PALInterface>()>> factories;

		PALModule(dynalo::library&& l, const std::string& id)
		  : lib(std::move(l)), id(id), factories(lib.get_function<std::map<FactoryType, std::function<std::shared_ptr<PALInterface>()>>()>("_CacaoPALModule_Factories")()) {}
	};

	class CACAO_API PALInterface {
	  public:
		virtual ~PALInterface() {}

	  protected:
		PALInterface() {}

		//We use the module for a reference count so it can't be unloaded while objects are alive
		std::shared_ptr<PALModule> m;
	};
}

#define PAL_BACKED_IMPL(c)                             \
	struct Cacao::c::Impl {                            \
		std::shared_ptr<Cacao::PAL##c##Interface> pal; \
	};
