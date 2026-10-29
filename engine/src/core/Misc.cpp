//This file is for things that are small enough not to warrant their own file so they go here instead

#include "Cacao/Mesh.hpp"
#include "Cacao/Script.hpp"
#include "ImplAccessor.hpp"
#include "SingletonGet.hpp"

namespace Cacao {
	ImplAccessor::ImplAccessor() {}

	CACAOST_GET(ImplAccessor)

	Script::~Script() = default;
	astra::TypeId Cacao::Script::ASTRA__gettypeid() const {
		return astra::TypeIdFactory<Cacao::Script>::get();
	}
}