#pragma once

#include "Entity.hpp"
#include "3D/Skybox.hpp"
#include "Graphics/Cameras/Camera.hpp"

#include <optional>

namespace Cacao {
	//Represents a world
	struct World {
	public:
		//Entities at the top level of the world
		std::vector<Entity> topLevelEntities;

		//Optional skybox
		std::optional<Skybox*> skybox;
	};
}