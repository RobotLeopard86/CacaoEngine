#include "Cacao/Tex2D.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/ThreadPool.hpp"
#include "impl/Tex2D.hpp"
#include "PALConfigurables.hpp"
#include "libcacaoimage.hpp"

#include <future>

namespace Cacao {
	Tex2D::Tex2D(libcacaoimage::Image&& imageBuffer, const std::string& addr)
	  : Asset(addr) {
		Check<BadValueException>(ValidateResourceAddr<Tex2D>(addr), "Resource address is malformed!");
		Check<BadValueException>(!imageBuffer.data.empty(), "Cannot construct a sound with an empty image buffer!");

		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Fill data
		impl->img = (imageBuffer.bitsPerChannel == 16 ? libcacaoimage::Convert16To8BitColor(imageBuffer) : std::move(imageBuffer));
	}

	Tex2D::~Tex2D() {
		if(realized) DropRealized();
	}

	Tex2D::Tex2D(Tex2D&& other)
	  : Asset(other.address) {
		//Steal the implementation pointer
		impl = std::move(other.impl);

		//Copy realization state
		realized = other.realized;
		other.realized = false;

		//Blank out other asset address
		other.address = "";
	}

	Tex2D& Tex2D::operator=(Tex2D&& other) {
		//Implementation pointer
		impl = std::move(other.impl);

		//Realization state
		realized = other.realized;
		other.realized = false;

		//Asset address
		address = other.address;
		other.address = "";

		return *this;
	}

	void Tex2D::Realize() {
		Check<BadRealizeStateException>(!realized, "Cannot realize a realized texture!");

		if(impl->DoWaitAsyncForSync()) {
			impl->Realize(realized).value().get();
		} else {
			impl->Realize(realized);
		}
	}

	std::shared_future<void> Tex2D::RealizeAsync() {
		Check<BadRealizeStateException>(!realized, "Cannot realize a realized texture!");

		if(impl->DoWaitAsyncForSync()) {
			return impl->Realize(realized).value();
		} else {
			return ThreadPool::Get().Exec([this](void) { this->impl->Realize(realized); });
		}
	}

	void Tex2D::DropRealized() {
		Check<BadRealizeStateException>(realized, "Cannot drop the realized representation of an unrealized texture; it does not exist!");

		realized = false;
		impl->DropRealized();
	}
}