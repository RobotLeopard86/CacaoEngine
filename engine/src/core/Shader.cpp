#include "Cacao/Shader.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "impl/Shader.hpp"
#include "PALConfigurables.hpp"

namespace Cacao {
	Shader::Shader(std::vector<unsigned char>&& shaderIR, uint8_t flags, const std::string& addr)
	  : Asset(addr) {
		Check<BadValueException>(ValidateResourceAddr<Shader>(addr), "Resource address is malformed!");
		Check<BadValueException>(!shaderIR.empty(), "Cannot construct a shader with empty data!");

		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Fill data
		impl->irBuffer = std::move(shaderIR);
		impl->flags = flags;
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