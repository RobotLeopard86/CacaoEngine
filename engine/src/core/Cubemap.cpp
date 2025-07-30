#include "Cacao/Cubemap.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/ThreadPool.hpp"
#include "impl/Cubemap.hpp"
#include "PALConfigurables.hpp"

#include <future>

namespace Cacao {
	Cubemap::Cubemap(std::array<libcacaoimage::Image, 6>&& faces, const std::string& addr)
	  : Asset(addr) {
		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Fill data
		impl->faces = faces;
	}

	Cubemap::~Cubemap() {
		if(realized) DropRealized();
	}

	void Cubemap::Realize() {
		Check<BadRealizeStateException>(!realized, "Cannot realize a realized cubemap!");

		if(impl->DoWaitAsyncForSync()) {
			impl->Realize(realized).value().get();
		} else {
			impl->Realize(realized);
		}
	}

	std::shared_future<void> Cubemap::RealizeAsync() {
		Check<BadRealizeStateException>(!realized, "Cannot realize a realized cubemap!");

		if(impl->DoWaitAsyncForSync()) {
			return impl->Realize(realized).value();
		} else {
			return ThreadPool::Get().Exec([this](void) { this->impl->Realize(realized); });
		}
	}

	void Cubemap::DropRealized() {
		Check<BadRealizeStateException>(realized, "Cannot drop the realized representation of an unrealized cubemap; it does not exist!");

		realized = false;
		impl->DropRealized();
	}
}