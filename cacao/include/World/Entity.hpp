#pragma once

#include <vector>
#include <optional>

#include "3D/Mesh.hpp"
#include "Graphics/Material.hpp"
#include "3D/Transform.hpp"
#include "Scripts/Script.hpp"
#include "Core/Log.hpp"

namespace Cacao {
	//A component on an entity
	class Component {
	protected:
		//Is this component active
		//Protected so subclasses can implement logic regarding this
		//The value of this boolean should be ignored if the entity active state is false
		bool active;
	};

	//An object in the world
	class Entity {
	public:
		Transform transform;
		std::vector<Component> components;

		bool active;
	};
}