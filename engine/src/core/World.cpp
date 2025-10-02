#include "Cacao/World.hpp"
#include "Cacao/Actor.hpp"
#include "Cacao/CodeRegistry.hpp"
#include "Cacao/Component.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/PerspectiveCamera.hpp"
#include "Cacao/Resource.hpp"
#include "Cacao/ResourceManager.hpp"
#include "Cacao/WorldManager.hpp"
#include "impl/ResourceManager.hpp"
#include "SingletonGet.hpp"
#include "ImplAccessor.hpp"

#include "libcacaoformats.hpp"

#include <memory>

namespace Cacao {
	World::World(const std::string& addr)
	  : Resource(addr) {
		Check<BadValueException>(ValidateResourceAddr<World>(addr), "Resource address is malformed!");

		//Create root actor
		root.actor = std::shared_ptr<Actor>(new Actor("__WORLDROOT__", ActorHandle {}, xg::Guid {}));
		root->isRoot = true;

		//Create camera
		cam = std::make_shared<PerspectiveCamera>();
		cam->SetPosition(glm::vec3 {0});
		cam->SetRotation(glm::vec3 {0});
	}

	std::shared_ptr<World> World::Create(const std::string& addr, const libcacaoformats::World& world) {
		//Create base world
		std::shared_ptr<World> w = Create(addr);

		//Configure camera and skybox
		w->cam->SetPosition({world.initialCamPos.x, world.initialCamPos.y, world.initialCamPos.z});
		w->cam->SetRotation({world.initialCamRot.x, world.initialCamRot.y, world.initialCamRot.z});
		if(!world.skyboxRef.empty() && ValidateResourceAddr<World>(world.skyboxRef)) {
			w->skyboxTex = *ResourceManager::Get().Load<Cubemap>(world.skyboxRef);
		}

		//Process actors and make tree
		std::map<xg::Guid, ActorHandle> foundActors;
		std::map<xg::Guid, std::vector<libcacaoformats::World::Actor>> awaitingParents;
		const auto processActor = [w, &foundActors, &awaitingParents](const libcacaoformats::World::Actor& actor) {
			auto impl = [w, &foundActors, &awaitingParents](const libcacaoformats::World::Actor& actor, auto& iref) mutable {
				//Generate handle
				ActorHandle hnd;
				if(actor.parentGUID == xg::Guid {}) {
					//Top-level actor
					hnd = Actor::Create(actor.name, w, actor.guid);
				} else if(foundActors.contains(actor.parentGUID)) {
					//The parent has been added to the tree
					hnd = Actor::Create(actor.name, foundActors[actor.parentGUID], actor.guid);
				} else {
					//The parent has not been added to the tree but we'll save this for when it is
					awaitingParents[actor.parentGUID].push_back(actor);
					return;
				}

				//Register actor object
				foundActors.insert_or_assign(hnd->guid, hnd);

				//Setup transform
				hnd->transform.SetPosition({actor.initialPos.x, actor.initialPos.y, actor.initialPos.z});
				hnd->transform.SetRotation({actor.initialRot.x, actor.initialRot.y, actor.initialRot.z});
				hnd->transform.SetScale({actor.initialScale.x, actor.initialScale.y, actor.initialScale.z});

				//Mount components
				for(const libcacaoformats::World::Component& comp : actor.components) {
					//Ensure the type is in the code registry
					Check<NonexistentValueException>(CodeRegistry::Get().HasFactory<Component>(comp.typeID), "World contains component of an unknown type! Hint: all component types must be registered in the CodeRegistry.");

					//Create the component
					hnd->MountComponent(comp.typeID);

					//TODO: Add the reflection data back in somehow
				}

				//Process components that should be children of this one
				if(awaitingParents.contains(actor.guid))
					for(const libcacaoformats::World::Actor& ca : awaitingParents[actor.guid]) iref(ca, iref);
			};
			impl(actor, impl);
		};
		for(const libcacaoformats::World::Actor& actor : world.actors) processActor(actor);

		//Return built world
		return w;
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