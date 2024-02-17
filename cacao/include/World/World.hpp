#pragma once

#include "Entity.hpp"
#include "3D/Skybox.hpp"
#include "Utilities/Tree.hpp"

#include <optional>

namespace Cacao {
	//Represents a world
	struct World {
	public:
		//Entities at the top level of the world
		Tree<Entity> worldTree;

		//Optional skybox
		std::optional<Skybox*> skybox;
	};
}