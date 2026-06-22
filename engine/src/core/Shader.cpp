#include "Cacao/Shader.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "impl/Shader.hpp"
#include "impl/ResourceManager.hpp"
#include "ImplAccessor.hpp"
#include "PALConfigurables.hpp"

namespace Cacao {
	Shader::Shader(std::vector<unsigned char>&& shaderIR, libcacaoasset::Shader::Descriptor desc, const std::string& addr)
	  : Asset(addr) {
		Check<BadValueException>(ValidateResourceAddr<Shader>(addr), "Resource address is malformed!");
		Check<BadValueException>(!shaderIR.empty(), "Cannot construct a shader with empty data!");

		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Fill data
		impl->irBuffer = std::move(shaderIR);
		impl->description = desc;
	}

	std::shared_ptr<Shader> Shader::Create(std::vector<unsigned char>&& shaderIR, libcacaoasset::Shader::Descriptor desc, const std::string& addr) {
		std::shared_ptr<Shader> ptr(new Shader(std::move(shaderIR), desc, addr));
		IMPL(ResourceManager).cache.insert_or_assign(addr, ptr);
		return ptr;
	}

	Shader::~Shader() {
		if(realized) DropRealized();
	}

	Shader::Shader(Shader&& other)
	  : Asset(other.address) {
		//Steal the implementation pointer
		impl = std::move(other.impl);

		//Copy realization state
		realized = other.realized;
		other.realized = false;

		//Blank out other asset address
		other.address = "";
	}

	Shader& Shader::operator=(Shader&& other) {
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

	void Shader::Realize() {
		Check<BadRealizeStateException>(!realized, "Cannot realize a realized shader!");

		impl->Realize(realized);
	}

	void Shader::DropRealized() {
		Check<BadRealizeStateException>(realized, "Cannot drop the realized representation of an unrealized shader; it does not exist!");

		realized = false;
		impl->DropRealized();
	}
}