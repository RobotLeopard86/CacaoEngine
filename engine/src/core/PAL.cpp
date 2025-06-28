#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "PALCommon.hpp"
#include "ImplAccessor.hpp"
#include "SingletonGet.hpp"

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

	void PAL::DestroyMP() {
		Check<PALModule, NonexistentValueException>(impl->mod, "Cannot destroy a null module!");
		impl->mod->Destroy();
		impl->mod.reset();
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

	void PAL::SetVSync(bool newState) {
		Check<BadInitStateException>(impl->mod->Connected(), "Cannot set V-Sync state with an unconnected module!");
		impl->mod->SetVSync(newState);
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

	ImplAccessor::ImplAccessor() {}

	CACAOST_GET(ImplAccessor)
	CACAOST_GET(PAL)
}
