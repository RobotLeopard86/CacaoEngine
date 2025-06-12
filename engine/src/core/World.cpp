#include "Cacao/World.hpp"

namespace Cacao {
	World::World(const std::string& addr, const std::string& pkg)
	  : Resource(addr, pkg) {
		//Create root actor
		root = Actor::Create("__WORLDROOT__", std::nullopt);
	}

	World::~World() {
		//Unwind the actor tree to break all references to avoid memory leaks
		//If there are outstanding references, then memory leaks still may occur... but not sure what I can do about that

		//This is a sneaky recursive lambda trick
		auto unwind = [](std::shared_ptr<Actor> a) {
			auto impl = [](std::shared_ptr<Actor> a, auto& implRef) mutable {
				//If this isn't here, the compiler throws a tantrum. Eh, it'll get optimized out (probably)
				if(false) return;

				//Unwind children
				for(std::shared_ptr<Actor> child : a->GetAllChildren()) {
					implRef(child, implRef);
				}

				//Now release pointer to this asset
				a.reset();
			};
			impl(a, impl);
		};

		//Execute the script locator
		unwind(root);
	}

	void World::ReparentToRoot(std::shared_ptr<Actor> actor) {
		actor->Reparent(root);
	}

	std::vector<std::shared_ptr<Actor>> World::GetRootChildren() {
		return root->GetAllChildren();
	}
}