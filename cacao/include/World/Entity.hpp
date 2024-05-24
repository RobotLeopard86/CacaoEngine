#pragma once

#include <vector>
#include <optional>
#include <random>

#include "3D/Mesh.hpp"
#include "Graphics/Material.hpp"
#include "3D/Transform.hpp"
#include "Scripts/Script.hpp"
#include "Component.hpp"
#include "Core/Log.hpp"

#include <memory>
#include <string>

#include "uuid_v4.h"

namespace Cacao {
	//An object in the world
	class Entity {
	  public:
		//UUID
		const UUIDv4::UUID uuid;

		//Components on this entity
		std::vector<std::shared_ptr<Component>> components;

		//Child entities
		std::vector<std::shared_ptr<Entity>> children;

		Transform transform;

		//Is this entity active?
		bool active;

		//Human-readable name
		std::string name;

		Entity(std::string name)
		  : uuid(UUIDv4::UUIDGenerator<std::mt19937_64>().getUUID()), transform(glm::vec3 {0}, glm::vec3 {0}, glm::vec3 {1}), active(true), name(name) {}
	};
}