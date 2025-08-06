#include "Cacao/Cubemap.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/ThreadPool.hpp"
#include "impl/Cubemap.hpp"
#include "PALConfigurables.hpp"

#include "libcacaoimage.hpp"

#include <future>

namespace Cacao {
	Cubemap::Cubemap(std::array<libcacaoimage::Image, 6>&& faces, const std::string& addr)
	  : Asset(addr) {
		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Validate images and do conversion if needed
		std::array<libcacaoimage::Image, 6> images = std::move(faces);
		for(uint8_t i = 0; i < 6; ++i) {
			Check<BadValueException>(images[i].layout == libcacaoimage::Image::Layout::RGB, "Only RGB images are supported for cubemaps!");
			if(images[i].bitsPerChannel == 16) images[i] = libcacaoimage::Convert16To8BitColor(images[i]);
		}

		//Fill data
		impl->faces = std::move(images);
	}

	Cubemap::~Cubemap() {
		if(realized) DropRealized();
	}

	Cubemap::Cubemap(Cubemap&& other)
	  : Asset(other.address) {
		//Steal the implementation pointer
		impl = std::move(other.impl);

		//Copy realization state
		realized = other.realized;
		other.realized = false;

		//Blank out other asset address
		other.address = "";
	}

	Cubemap& Cubemap::operator=(Cubemap&& other) {
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