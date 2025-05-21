#include "Cacao/World.hpp"

namespace Cacao {
	World::World() {
		//Create root actor
		root = Actor::Create("__WORLDROOT__", std::nullopt);
	}

	void World::ReparentToRoot(std::shared_ptr<Actor> actor) {
		actor->Reparent(root);
	}

	std::vector<std::shared_ptr<Actor>> World::GetRootChildren() {
		return root->GetAllChildren();
	}
}