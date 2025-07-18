//This file is for things that are small enough not to warrant their own file so they go here instead

#include "Cacao/Component.hpp"
#include "Cacao/Mesh.hpp"
#include "ImplAccessor.hpp"
#include "SingletonGet.hpp"

namespace Cacao {
	std::shared_ptr<Component> ComponentExporter::Instantiate() {
		std::shared_ptr<Component> c = factory();
		c->expHnd = std::enable_shared_from_this<ComponentExporter>::shared_from_this();
		return c;
	}

	ImplAccessor::ImplAccessor() {}

	CACAOST_GET(ImplAccessor)
}