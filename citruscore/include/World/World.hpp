#pragma once

#include "Entity.hpp"
#include "3D/Skybox.hpp"

#include <optional>

namespace Citrus {
	//Represents a world
	struct World {
	public:
		//Entities at the top level of the world
		std::vector<Entity> topLevelEntities;

		//Optional pointer to a skybox
		std::optional<Skybox*> skybox;
	};
}