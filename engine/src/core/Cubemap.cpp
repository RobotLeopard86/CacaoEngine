#include "Cacao/Cubemap.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "impl/Cubemap.hpp"
#include "impl/ResourceManager.hpp"
#include "ImplAccessor.hpp"
#include "PALConfigurables.hpp"

#include "libcacaoimage.hpp"

namespace Cacao {
	Cubemap::Cubemap(std::array<libcacaoimage::Image, 6>&& faces, const std::string& addr)
	  : Asset(addr) {
		Check<BadValueException>(ValidateResourceAddr<Cubemap>(addr), "Resource address is malformed!");

		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Validate images and do conversion if needed
		std::array<libcacaoimage::Image, 6> images = std::move(faces);
		glm::uvec2 refSz = {0, 0};
		for(uint8_t i = 0; i < 6; ++i) {
			//Ensure same sizes
			if(refSz.x == 0 && refSz.y == 0)
				refSz = {images[i].w, images[i].h};
			else
				Check<BadValueException>(images[i].w == refSz.x && images[i].h == refSz.y, "Not all cubemap faces are the same size!");

			//Convert to 8-bit
			if(images[i].bitsPerChannel == 16) images[i] = libcacaoimage::Convert16To8BitColor(images[i]);

			//Ensure RGB layout
			if(images[i].layout != libcacaoimage::Image::Layout::RGB) images[i] = libcacaoimage::ChangeChannelLayout(images[i], libcacaoimage::Image::Layout::RGB);
		}

		//Fill data
		impl->faces = std::move(images);
	}

	std::shared_ptr<Cubemap> Cubemap::Create(std::array<libcacaoimage::Image, 6>&& faces, const std::string& addr) {
		std::shared_ptr<Cubemap> ptr(new Cubemap(std::move(faces), addr));
		std::lock_guard lk {IMPL(ResourceManager).cacheProtector};
		IMPL(ResourceManager).cache.insert_or_assign(addr, ptr);
		return ptr;
	}

	Cubemap::~Cubemap() {
		if(baked) Discard();
	}

	Cubemap::Cubemap(Cubemap&& other)
	  : Asset(other.address) {
		//Steal the implementation pointer
		impl = std::move(other.impl);

		//Copy baking state
		baked = other.baked;
		other.baked = false;

		//Blank out other asset address
		other.address = "";
	}

	Cubemap& Cubemap::operator=(Cubemap&& other) {
		//Implementation pointer
		impl = std::move(other.impl);

		//Baking state
		baked = other.baked;
		other.baked = false;

		//Asset address
		address = other.address;
		other.address = "";

		return *this;
	}

	void Cubemap::Bake() {
		Check<BadBakeStateException>(!baked, "Cannot bake a baked cubemap!");

		impl->Bake(baked);
	}

	void Cubemap::Discard() {
		Check<BadBakeStateException>(baked, "Cannot discard baked representation of an unbaked cubemap; it does not exist!");

		baked = false;
		impl->Discard();
	}
}