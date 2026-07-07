#include "Cacao/Tex2D.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "impl/Tex2D.hpp"
#include "impl/ResourceManager.hpp"
#include "ImplAccessor.hpp"
#include "PALConfigurables.hpp"

#include "libcacaoimage.hpp"

namespace Cacao {
	Tex2D::Tex2D(libcacaoimage::Image&& imageBuffer, const std::string& addr)
	  : Asset(addr) {
		Check<BadValueException>(ValidateResourceAddr<Tex2D>(addr), "Resource address is malformed!");
		Check<BadValueException>(!imageBuffer.data.empty(), "Cannot construct a sound with an empty image buffer!");

		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Fill data
		impl->img = (imageBuffer.bitsPerChannel == 16 ? libcacaoimage::Convert16To8BitColor(imageBuffer) : std::move(imageBuffer));
		if(impl->img.layout == libcacaoimage::Image::Layout::RGB) impl->img = libcacaoimage::ChangeChannelLayout(impl->img, libcacaoimage::Image::Layout::RGBA);
	}

	std::shared_ptr<Tex2D> Tex2D::Create(libcacaoimage::Image&& imageBuffer, const std::string& addr) {
		std::shared_ptr<Tex2D> ptr(new Tex2D(std::move(imageBuffer), addr));
		IMPL(ResourceManager).cache.insert_or_assign(addr, ptr);
		return ptr;
	}

	Tex2D::~Tex2D() {
		if(baked) Discard();
	}

	Tex2D::Tex2D(Tex2D&& other)
	  : Asset(other.address) {
		//Steal the implementation pointer
		impl = std::move(other.impl);

		//Copy baking state
		baked = other.baked;
		other.baked = false;

		//Blank out other asset address
		other.address = "";
	}

	Tex2D& Tex2D::operator=(Tex2D&& other) {
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

	void Tex2D::Bake() {
		Check<BadBakeStateException>(!baked, "Cannot bake a baked texture!");

		impl->Bake(baked);
	}

	void Tex2D::Discard() {
		Check<BadBakeStateException>(baked, "Cannot discard baked representation of an unbaked texture; it does not exist!");

		baked = false;
		impl->Discard();
	}
}