#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/Mesh.hpp"
#include "Cacao/Shader.hpp"
#include "impl/PAL.hpp"
#include "SingletonGet.hpp"
#include "PALConfigurables.hpp"

namespace Cacao {
	PAL::PAL() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	PAL::~PAL() {
		impl->mod.reset();
	}

	void PAL::SetModule(const std::string& mod) {
		Check<MiscException>(impl->mod.use_count() <= 1, "Cannot change the module when objects referencing the old one still exist!");
		Check<NonexistentValueException>(impl->registry.contains(mod), "The requested module does not exist!");
		impl->mod = impl->registry[mod]();
	}

	bool PAL::InitializeModule() {
		Check<PALModule, NonexistentValueException>(impl->mod, "Cannot initialize a null module!");
		Check<BadInitStateException>(!impl->mod->Initialized(), "Cannot initialize an already initialized module!");
		try {
			impl->mod->Init();
			return true;
		} catch(...) {
			return false;
		}
	}

	void PAL::GfxConnect() {
		Check<PALModule, NonexistentValueException>(impl->mod, "Cannot connect a null module!");
		Check<BadInitStateException>(impl->mod->Initialized(), "Cannot connect an uninitialized module!");
		Check<BadInitStateException>(!impl->mod->Connected(), "Cannot connect an already connected module!");
		impl->mod->Connect();
	}

	void PAL::GfxDisconnect() {
		Check<BadInitStateException>(impl->mod->Connected(), "Cannot disconnect an unconnected module!");
		impl->mod->Disconnect();
	}

	void PAL::TerminateModule() {
		Check<BadInitStateException>(impl->mod->Initialized(), "Cannot terminate an uninitialized module!");
		Check<BadInitStateException>(!impl->mod->Connected(), "Cannot terminate a connected module!");
		impl->mod->Term();
	}

#define CONFIGURE_IMPLPTR(t)                                                                                                                 \
	template<>                                                                                                                               \
	void PAL::ConfigureImplPtr<t>(t & obj) {                                                                                                 \
		Check<PALModule, NonexistentValueException>(impl->mod, "Cannot configure an implementation pointer using a null module!");           \
		Check<BadInitStateException>(impl->mod->Initialized(), "Cannot configure an implementation pointer using an uninitialized module!"); \
		obj.impl.reset(impl->mod->Configure##t());                                                                                           \
	}

	CONFIGURE_IMPLPTR(Mesh)
	CONFIGURE_IMPLPTR(Tex2D)
	CONFIGURE_IMPLPTR(Cubemap)
	CONFIGURE_IMPLPTR(GPUManager)
	CONFIGURE_IMPLPTR(Shader)
#undef CONFIGURE_IMPLPTR

	CACAOST_GET(PAL)
}
