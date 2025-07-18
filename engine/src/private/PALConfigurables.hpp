#pragma once

#include "Cacao/PAL.hpp"
#include "Cacao/Mesh.hpp"

namespace Cacao {
	template<>
	void PAL::ConfigureImplPtr<Mesh>(Mesh&);
}