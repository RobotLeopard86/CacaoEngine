#pragma once

#include <vector>
#include <optional>

#include "3D/Mesh.hpp"
#include "Graphics/Material.hpp"
#include "3D/Transform.hpp"
#include "Scripts/Script.hpp"
#include "Core/Log.hpp"

namespace Cacao {
	//An object in the world
	class Entity {
	public:
		Transform transform;
		std::vector<Entity> children;
		std::optional<Mesh> mesh;
		std::optional<Material> mat;
		/*
		These are things that will be supported in the future but currently aren't
		std::optional<Collider> collider;
		std::optional<PhysicsBody> physBody;
		*/

		bool active;
	};

	//Script with a built-in entity target
	class EntityScript : public Script {
	public:
		EntityScript(Entity& e)
			: target(e) {}
	protected:
		const Entity& target;
	};
}