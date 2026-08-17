#include "Cacao/World.hpp"
#include "Cacao/Actor.hpp"
#include "Cacao/CodeRegistry.hpp"
#include "Cacao/EventConsumer.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/OrthographicCamera.hpp"
#include "Cacao/PerspectiveCamera.hpp"
#include "Cacao/Resource.hpp"
#include "Cacao/ResourceManager.hpp"
#include "Cacao/WorldManager.hpp"
#include "crossguid/guid.hpp"
#include "impl/ResourceManager.hpp"
#include "impl/World.hpp"
#include "SingletonGet.hpp"
#include "ImplAccessor.hpp"

#include "libcacaoasset.hpp"

#include "astra/var.hpp"
#include "astra/binary.hpp"

#include <memory>
#include <ranges>

namespace Cacao {
	World::World(bool useOrthographicCamera, const std::string& addr)
	  : Resource(addr) {
		Check<BadValueException>(ValidateResourceAddr<World>(addr), "Resource address is malformed!");

		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//Create camera
		if(useOrthographicCamera) {
			cam = std::make_shared<OrthographicCamera>();
		} else {
			cam = std::make_shared<PerspectiveCamera>();
		}
		cam->SetPosition(glm::vec3 {0});
		cam->SetRotation(glm::vec3 {0});
	}

	std::shared_ptr<World> World::Create(bool useOrthographicCamera, const std::string& addr) {
		std::shared_ptr<World> ptr(new World(useOrthographicCamera, addr));
		std::lock_guard lk {IMPL(ResourceManager).cacheProtector};
		IMPL(ResourceManager).cache.insert_or_assign(addr, ptr);
		return ptr;
	}

	std::shared_ptr<World> World::Create(const libcacaoasset::World& world, const std::string& addr) {
		//Create base world
		std::shared_ptr<World> w = Create(world.camOrthographic, addr);

		//Configure camera and skybox
		w->cam->SetPosition({world.initialCamPos.x, world.initialCamPos.y, world.initialCamPos.z});
		w->cam->SetRotation({world.initialCamRot.x, world.initialCamRot.y, world.initialCamRot.z, world.initialCamRot.w});
		w->cam->SetClearColor({world.camClearColor.r, world.camClearColor.g, world.camClearColor.b});
		if(!world.skybox.empty() && ValidateResourceAddr<Cubemap>(world.skybox)) {
			w->skyboxTex = *ResourceManager::Get().Load<Cubemap>(world.skybox);
		}

		//Create fake parent ref for toplevel actors
		ActorRef fakeParent;
		fakeParent.world = w;

		//Process actors and make tree
		std::unordered_map<xg::Guid, ActorRef> foundActors;
		std::unordered_map<xg::Guid, std::vector<libcacaoasset::World::Actor>> awaitingParents;
		const auto processActor = [w, &foundActors, &awaitingParents, &fakeParent](const libcacaoasset::World::Actor& actor) {
			auto impl = [w, &foundActors, &awaitingParents, &fakeParent](const libcacaoasset::World::Actor& actor, auto& iref) mutable {
				//Generate handle
				ActorRef ref;
				xg::Guid pguid(actor.parentGUID);
				xg::Guid guid(actor.guid);
				if(pguid == xg::Guid {}) {
					//Top-level actor
					Impl::ActorSlot& slot = w->impl->slotTable.emplace_back(Impl::ActorSlot {.generation = 1, .id = w->impl->slotTable.size(), .actor = std::unique_ptr<Actor> {}});
					slot.actor = std::unique_ptr<Actor>(new Actor(actor.name, fakeParent, guid));
					ref = ActorRef(w, w->impl->slotTable.size() - 1, 1);
				} else if(foundActors.contains(pguid)) {
					//The parent has been added to the tree
					Impl::ActorSlot& slot = w->impl->slotTable.emplace_back(Impl::ActorSlot {.generation = 1, .id = w->impl->slotTable.size(), .actor = std::unique_ptr<Actor> {}});
					slot.actor = std::unique_ptr<Actor>(new Actor(actor.name, foundActors[pguid], guid));
					ref = ActorRef(w, w->impl->slotTable.size() - 1, 1);
				} else {
					//The parent has not been added to the tree but we'll save this for when it is
					awaitingParents[pguid].push_back(actor);
					return;
				}

				//Register actor object
				foundActors.insert_or_assign(ref->guid, ref);

				//Setup transform and state
				ref->SetActive(actor.active);
				ref->transform.SetPosition({actor.initialPos.x, actor.initialPos.y, actor.initialPos.z});
				ref->transform.SetRotation({actor.initialRot.x, actor.initialRot.y, actor.initialRot.z, actor.initialRot.w});
				ref->transform.SetScale({actor.initialScale.x, actor.initialScale.y, actor.initialScale.z});

				//Mount components
				for(const libcacaoasset::World::Component& comp : actor.components) {
					//Ensure the type is in the code registry
					Check<NonexistentValueException>(CodeRegistry::Get().HasFactory<Component>(comp.typeID), "World contains component of an unknown type! Hint: all component types must be registered in the CodeRegistry.");

					//Create the component
					Component& component = ref->MountComponent(comp.typeID, comp.enabled);

					//Inject reflected data
					astra::Var cvar(&component);
					astra::binary::fromVectorIntoVar(comp.reflection, cvar);
				}

				//Process components that should be children of this one
				if(awaitingParents.contains(guid))
					for(const libcacaoasset::World::Actor& ca : awaitingParents[guid]) iref(ca, iref);
			};
			impl(actor, impl);
		};
		for(const libcacaoasset::World::Actor& actor : world.actors) processActor(actor);

		//Return built world
		return w;
	}

	World::~World() {}

	void World::MakeToplevel(ActorRef actor) {
		Check<BadValueException>(actor->world != this, "Cannot make an actor that does not belong to this world toplevel!");

		//Remove actor from its current parent
		auto common = actor->parent->children | std::views::filter([&actor](const ActorRef& ref) {
			return ref == actor;
		}) | std::views::common;
		actor->parent->children = std::vector<ActorRef>(common.begin(), common.end());

		//Set parent to null (marks it as toplevel)
		actor->parent = ActorRef {};
		actor->parent.world = std::static_pointer_cast<World>(shared_from_this());
	}

	std::vector<ActorRef> World::GetToplevelActors() const {
		auto common = impl->slotTable | std::views::transform([this](const Impl::ActorSlot& slot) {
			return ActorRef(std::const_pointer_cast<World>(std::static_pointer_cast<const World>(shared_from_this())), slot.id, slot.generation);
		}) | std::views::filter([](const ActorRef& ref) {
			return !((bool)ref->parent);
		}) | std::views::common;
		return std::vector<ActorRef>(common.begin(), common.end());
	}

	std::vector<ActorRef> World::_FindActors(std::function<bool(ActorRef)> predicate) const {
		auto common = impl->slotTable | std::views::transform([this](const Impl::ActorSlot& slot) {
			return ActorRef(std::const_pointer_cast<World>(std::static_pointer_cast<const World>(shared_from_this())), slot.id, slot.generation);
		}) | std::views::filter(predicate) |
					  std::views::common;
		return std::vector<ActorRef>(common.begin(), common.end());
	}

	struct WorldManager::Impl {
		std::shared_ptr<World> active;
		EventConsumer shutdownConsumer;
	};

	WorldManager::WorldManager() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//Register active world release on shutdown
		impl->shutdownConsumer = EventConsumer([this](Event&) {
			impl->active.reset();
			EventManager::Get().UnsubscribeConsumer("EngineShutdown", impl->shutdownConsumer);
		});
		EventManager::Get().SubscribeConsumer("EngineShutdown", impl->shutdownConsumer);
	}

	WorldManager::~WorldManager() {}

	CACAOST_GET(WorldManager)

	std::string WorldManager::GetActiveWorldAddr() {
		return impl->active ? impl->active->GetAddress() : "";
	}

	std::shared_ptr<World> WorldManager::GetActiveWorld() {
		return impl->active;
	}

	void WorldManager::SetActiveWorld(const std::string& addr, bool noload) {
		//Validate the resource address
		Check<BadValueException>(Resource::ValidateResourceAddr<World>(addr), "World address is malformed!");

		//Check resource cache
		std::unique_lock<std::mutex> lk {IMPL(ResourceManager).cacheProtector};
		if(!IMPL(ResourceManager).cache.contains(addr)) {
			//noload check
			Check<NonexistentValueException>(!noload, "World requested for activation is not loaded, and noload flag was specified!");

			//Load it
			lk.unlock();
			impl->active = *ResourceManager::Get().Load<World>(addr);
			return;
		} else {
			impl->active = std::static_pointer_cast<World>(IMPL(ResourceManager).cache[addr].lock());
		}
		lk.unlock();
	}

	ActorRef World::CreateActor(const std::string& name, ActorRef parent) {
		Check<NonexistentValueException>(!parent.null && parent, "Cannot create an actor under a nonexistent or expired parent!");

		//Obtain slot for actor creation
		Impl::ActorSlot& slot = [this]() -> Impl::ActorSlot& {
			if(impl->freeList.empty()) {
				Impl::ActorSlot& s = impl->slotTable.emplace_back();
				s.generation = 1;
				s.id = impl->slotTable.size() - 1;
				return s;
			} else {
				Impl::ActorSlot& s = impl->slotTable[*impl->freeList.begin()];
				impl->freeList.erase(impl->freeList.begin());
				++(s.generation);
				return s;
			}
		}();

		//Create actor in slot
		slot.actor = std::unique_ptr<Actor>(new Actor(name, parent, xg::newGuid()));
		return slot.actor->self;
	}
}