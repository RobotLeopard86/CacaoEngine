#include "Cacao/World.hpp"
#include "Cacao/Actor.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/PerspectiveCamera.hpp"
#include "Cacao/Resource.hpp"
#include "Cacao/ResourceManager.hpp"
#include "Cacao/WorldManager.hpp"
#include "impl/ResourceManager.hpp"
#include "SingletonGet.hpp"
#include "ImplAccessor.hpp"

#include <memory>

namespace Cacao {
	World::World(const std::string& addr)
	  : Resource(addr) {
		Check<BadValueException>(ValidateResourceAddr<World>(addr), "Resource address is malformed!");

		//Create root actor
		root.actor = std::shared_ptr<Actor>(new Actor("__WORLDROOT__", ActorHandle {}));
		root->isRoot = true;

		//Create camera
		cam = std::make_shared<PerspectiveCamera>();
		cam->SetPosition(glm::vec3 {0});
		cam->SetRotation(glm::vec3 {0});
	}

	World::~World() {}

	void World::ReparentToRoot(ActorHandle actor) {
		actor->Reparent(root);
	}

	std::vector<ActorHandle> World::GetRootChildren() const {
		return root->GetAllChildren();
	}

	struct WorldManager::Impl {
		std::shared_ptr<World> active;
	};

	WorldManager::WorldManager() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	WorldManager::~WorldManager() {}

	CACAOST_GET(WorldManager)

	std::string WorldManager::GetActiveWorld() {
		return impl->active ? impl->active->GetAddress() : "";
	}

	std::shared_ptr<World> WorldManager::operator()() {
		Check<World, NonexistentValueException>(impl->active, "No active world is set!");
		return impl->active;
	}

	void WorldManager::SetActiveWorld(const std::string& addr, bool noload) {
		//Validate the resource address
		Check<BadValueException>(Resource::ValidateResourceAddr<World>(addr), "World address is malformed!");

		//Check resource cache
		if(!IMPL(ResourceManager).cache.contains(addr)) {
			//noload check
			Check<NonexistentValueException>(noload, "World requested for activation is not loaded, and noload flag was specified!");

			//Load it
			impl->active = *ResourceManager::Get().Load<World>(addr);
		}
		impl->active = std::static_pointer_cast<World>(IMPL(ResourceManager).cache[addr].lock());
	}
}