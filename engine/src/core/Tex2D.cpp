#include "Cacao/Tex2D.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/ThreadPool.hpp"
#include "impl/Tex2D.hpp"
#include "PALConfigurables.hpp"

#include <future>

namespace Cacao {
	Tex2D::Tex2D(libcacaoimage::Image&& imageBuffer, const std::string& addr)
	  : Asset(addr) {
		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Fill data
		impl->img = imageBuffer;
	}

	Tex2D::~Tex2D() {
		if(realized) DropRealized();
	}

	void Tex2D::Realize() {
		Check<BadRealizeStateException>(!realized, "Cannot realize a realized texture!");

		if(impl->DoWaitAsyncForSync()) {
			impl->Realize().value().get();
		} else {
			impl->Realize();
		}
	}

	std::shared_future<void> Tex2D::RealizeAsync() {
		Check<BadRealizeStateException>(!realized, "Cannot realize a realized texture!");

		if(impl->DoWaitAsyncForSync()) {
			return impl->Realize().value();
		} else {
			return ThreadPool::Get().Exec([this](void) { this->impl->Realize(); });
		}
	}

	void Tex2D::DropRealized() {
		Check<BadRealizeStateException>(realized, "Cannot drop the realized representation of an unrealized texture; it does not exist!");

		realized = false;
		impl->DropRealized();
	}
}