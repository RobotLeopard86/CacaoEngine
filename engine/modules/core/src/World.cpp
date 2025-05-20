#include "Cacao/World.hpp"

namespace Cacao {
	World::World() {
		//Create root entity
		root = Entity::Create("__WORLDROOT__", std::nullopt);
	}

	void World::ReparentToRoot(std::shared_ptr<Entity> entity) {
		entity->Reparent(root);
	}

	std::vector<std::shared_ptr<Entity>> World::GetRootChildren() {
		return root->GetAllChildren();
	}
}