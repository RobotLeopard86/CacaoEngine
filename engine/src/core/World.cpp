#include "Cacao/World.hpp"
#include "Cacao/Actor.hpp"
#include "Cacao/PerspectiveCamera.hpp"
#include "Cacao/ResourceManager.hpp"

namespace Cacao {
	World::World(libcacaoformats::World&& world, const std::string& addr)
	  : Resource(addr) {
		Check<BadValueException>(ValidateResourceAddr<World>(addr), "Resource address is malformed!");

		//Create root actor
		root.actor = std::shared_ptr<Actor>(new Actor("__WORLDROOT__", ActorHandle {}));
		root->isRoot = true;

		//Create camera
		cam = std::make_shared<PerspectiveCamera>();
		cam->SetPosition({world.initialCamPos.x, world.initialCamPos.y, world.initialCamPos.z});
		cam->SetRotation({world.initialCamRot.x, world.initialCamRot.y, world.initialCamRot.z});

		//Skybox
		if(!world.skyboxRef.empty()) skyboxTex = ResourceManager::Get().Load<Cubemap>(world.skyboxRef).get();
	}

	World::~World() {}

	void World::ReparentToRoot(ActorHandle actor) {
		actor->Reparent(root);
	}

	std::vector<ActorHandle> World::GetRootChildren() const {
		return root->GetAllChildren();
	}
}