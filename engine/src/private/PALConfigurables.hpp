#pragma once

#include "Cacao/PAL.hpp"
#include "Cacao/Mesh.hpp"
#include "Cacao/Tex2D.hpp"
#include "Cacao/Cubemap.hpp"

namespace Cacao {
	template<>
	void PAL::ConfigureImplPtr<Mesh>(Mesh&);

	template<>
	void PAL::ConfigureImplPtr<Tex2D>(Tex2D&);

	template<>
	void PAL::ConfigureImplPtr<Cubemap>(Cubemap&);
}