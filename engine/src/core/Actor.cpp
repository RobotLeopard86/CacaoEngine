#include "Cacao/Actor.hpp"
#include "Cacao/CodeRegistry.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/World.hpp"
#include "impl/World.hpp"
#include "ImplAccessor.hpp"

#include "crossguid/guid.hpp"

#include <memory>

namespace Cacao {
	Actor::Actor(const std::string& name, ActorRef parent, xg::Guid guid)
	  : name(name), guid(guid), parent(parent), transform({0, 0, 0}, {0, 0, 0}, {1, 1, 1}), worldTransformCached(GetWorldTransform()), world(parent->world), active(true) {
		const World::Impl::ActorSlot& ourSlot = *std::find_if(IMPL(World, *world).slotTable.begin(), IMPL(World, *world).slotTable.end(), [&guid](const World::Impl::ActorSlot& slot) {
			return slot.actor->guid == guid;
		});
		self = ActorRef(std::static_pointer_cast<World>(world->shared_from_this()), ourSlot.id, ourSlot.generation);
	}

	glm::mat4 Actor::GetWorldTransformationMatrix() const {
		//Calculate the transformation matrix
		//This should take a (0, 0, 0) coordinate relative to the actor and turn it into a world space transform
		ActorRef current = self;
		glm::mat4 transMat = transform.GetTransformationMatrix();
		do {
			//Apply transformation
			transMat = current->transform.GetTransformationMatrix() * transMat;
		} while((current = current->GetParent()));

		return transMat;
	}

	Transform Actor::GetWorldTransform() const {
		if(!transformDirty) return worldTransformCached;

		//Calculate the parent chain up to the world root
		//This is because to apply transformations correctly, we have to go parent -> child
		std::vector<ActorRef> pchain;
		{
			ActorRef current = self;
			do {
				//Add parent to list
				pchain.push_back(current);
			} while((current = current->GetParent()));
		}

		//Now we go in reverse to actually apply the transformations
		Transform worldTransform({0, 0, 0}, {0, 0, 0}, {1, 1, 1});
		for(auto it = pchain.rbegin(); it != pchain.rend(); ++it) {
			//Get current transform
			Transform current = (*pchain.rbegin())->GetLocalTransform();

			//Apply transformations
			worldTransform.SetPosition(worldTransform.GetPosition() + worldTransform.GetRotation() * current.GetPosition());
			worldTransform.SetRotation(worldTransform.GetRotation() * current.GetRotation());
			worldTransform.SetScale(worldTransform.GetScale() * current.GetScale());
		}

		transformDirty = false;
		worldTransformCached = worldTransform;
		return worldTransformCached;
	}

	void Actor::Reparent(ActorRef newParent) {
		Check<BadValueException>((bool)newParent, "Cannot reparent an actor to a null reference!");
		Check<BadValueException>(newParent->guid != guid, "Cannot reparent an actor to itself!");

		//Remove ourselves from the current parent
		auto common = parent->children | std::views::filter([this](const ActorRef& ref) {
			return ref != self;
		}) | std::views::common;
		parent->children = std::vector<ActorRef>(common.begin(), common.end());

		//Add ourselves to new parent
		parent = newParent;
		parent->children.push_back(self);
	}

	Component& Actor::MountComponent(const std::string& factoryID) {
		//Try to create the object
		auto [ptr, type] = CodeRegistry::Get().Instantiate<Component>(factoryID);
		Check<ExistingValueException>(!components.contains(type), "A component of the type specified already exists on the actor!");

		//Call down to common component setup
		std::unique_ptr<Component> uptr(ptr);
		_ComponentSetup(type, std::move(uptr));

		//Return the component reference
		return *components[type].component;
	}

	void Actor::SetActive(bool state) {
		if(state == active) return;
		active = state;
	}

	void Actor::_ComponentSetup(std::type_index type, std::unique_ptr<Component>&& ptr) {
		//Get or create new handle
		ComponentHandle& handle = components[type];

		//Update generation number to invalidate old references
		++(handle.generation);

		//Store component pointer
		handle.component = std::move(ptr);

		//Set up component object
		handle.component->owner = self;
		handle.component->OnMount();
		handle.component->SetEnabled(true);
	}

	std::unordered_map<std::type_index, Component*> Actor::_ComponentGet(std::function<bool(const std::unique_ptr<Component>&)> filter) const {
		auto common = (components | std::views::filter([&filter](const typename decltype(components)::value_type& component) {
			if(!component.second.component) return false;
			return filter(component.second.component);
		}) | std::views::transform([](const typename decltype(components)::value_type& component) {
			return std::make_pair<std::type_index, Component*>(std::type_index(component.first), component.second.component.get());
		}) | std::views::common);
		return std::unordered_map<std::type_index, Component*>(common.begin(), common.end());
	}

	bool Component::IsEnabled() const {
		return enabled && owner->IsActive();
	}

	bool Actor::IsActive() const {
		return active && (parent ? parent->IsActive() : true);
	}

	ActorRef::ActorRef(std::weak_ptr<World> world, uint64_t slot, uint64_t generation) : world(world), slotID(slot), generation(generation), null(false) {}

	//Returns nullptr on non-valid handle
	void* ActorRef::Resolve() const noexcept {
		//Lock world
		std::shared_ptr<World> worldReal = world.lock();

		//Return nullptr if slot ID in free list (no actor there)
		if(IMPL(World, *worldReal).freeList.contains(slotID)) return nullptr;

		//Get the slot
		return &(IMPL(World, *worldReal).slotTable[slotID]);
	}

	ActorRef::operator bool() const noexcept {
		//Null handles and expired worlds mean we can't even search for the actor
		if(null || world.expired()) return false;

		//Try to obtain the slot, return false if our slot ID is in the free list (no actor there)
		void* maybeSlot = Resolve();
		if(!maybeSlot) return false;

		//Get the real slot type, and check it against what we know
		World::Impl::ActorSlot* slot = static_cast<World::Impl::ActorSlot*>(maybeSlot);
		return slot->actor && slot->generation == generation;
	}

	bool ActorRef::operator==(const ActorRef& rhs) const noexcept {
		//If both null, true
		if(null && rhs.null) return true;

		//If null-state mismatch, false
		if(null != rhs.null) return false;

		//Check slot ID and generation info (we do this before the world because to compare worlds we have to do more expensive operations, and comparing numbers is cheap)
		if(slotID != rhs.slotID || generation != rhs.generation) return false;

		//For handles with expired worlds, we can't check if they're the same world, so we have to return false
		if(world.expired() || rhs.world.expired()) return false;

		//Check if the worlds are the same
		std::shared_ptr<World> ours = world.lock();
		std::shared_ptr<World> theirs = rhs.world.lock();
		if(ours != theirs) return false;

		//All checks passed, handles must be equal!
		return true;
	}

	Actor* ActorRef::operator->() {
		Check<NonexistentValueException>(operator bool(), "Cannot dereference an invalid ActorRef!");

		//We can directly cast to ActorSlot* because operator bool() checks the safety of that pointer
		World::Impl::ActorSlot* slot = static_cast<World::Impl::ActorSlot*>(Resolve());
		return slot->actor.get();
	}

	const Actor* ActorRef::operator->() const {
		Check<NonexistentValueException>(operator bool(), "Cannot dereference an invalid ActorRef!");

		//We can directly cast to ActorSlot* because operator bool() checks the safety of that pointer
		const World::Impl::ActorSlot* slot = static_cast<const World::Impl::ActorSlot*>(Resolve());
		return slot->actor.get();
	}
}