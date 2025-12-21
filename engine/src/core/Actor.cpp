#include "Cacao/Actor.hpp"
#include "Cacao/CodeRegistry.hpp"
#include "Cacao/Component.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/World.hpp"
#include "crossguid/guid.hpp"

#include <algorithm>
#include <memory>

namespace Cacao {
	Actor::~Actor() {
		//Release ownership of all components
		for(auto comp : components) {
			comp.second.reset();
		}
		components.clear();

		//Release ownership of all children
		children.clear();

		//Reset pointers to avoid destructor issues
		parentPtr.reset();
	}

	glm::mat4 Actor::GetWorldTransformMatrix() const {
		//Calculate the transformation matrix
		//This should take a (0, 0, 0) coordinate relative to the actor and turn it into a world space transform
		ActorHandle current = [this]() {ActorHandle h; h.actor = std::const_pointer_cast<Actor>(shared_from_this()); h.world = world.lock(); return h; }();
		glm::mat4 transMat = transform.GetTransformationMatrix();
		while(!(current = current->GetParent()).IsNull()) {
			//Apply this transformation
			transMat = current->transform.GetTransformationMatrix() * transMat;
		}

		return transMat;
	}

	ActorHandle Actor::GetParent() const {
		ActorHandle hnd;
		if(isRoot || parentPtr.lock()->isRoot) return hnd;
		hnd.actor = parentPtr.lock();
		hnd.world = world.lock();
		return hnd;
	}

	void Actor::Reparent(ActorHandle newParent) {
		//Remove ourselves from the current parent
		std::shared_ptr<Actor> selfPtr = shared_from_this();
		std::shared_ptr<Actor> parent = parentPtr.lock();
		auto it = std::find_if(parent->children.begin(), parent->children.end(), [&selfPtr](std::shared_ptr<Actor> a) {
			return a == selfPtr;
		});
		if(it != parent->children.end()) parent->children.erase(it);

		//Make sure we aren't parenting to ourselves
		Check<BadValueException>(newParent.actor != selfPtr, "Cannot parent an Actor to itself!");

		//Add ourselves as a child to the new parent
		newParent->children.push_back(shared_from_this());

		//Set parent pointer
		parentPtr = newParent.actor;
	}

	void Actor::MountComponent(const std::string& factoryID) {
		//Try to create the object
		auto [ptr, type] = CodeRegistry::Get().Instantiate<Component>(factoryID);

		//Add it
		Check<ExistingValueException>(!components.contains(type), "A component of the type specified already exists on the actor!");
		components.insert_or_assign(type, ptr);
		PostMountComponent(components[type]);
	}


	void Actor::PostMountComponent(std::shared_ptr<Component> c) {
		c->actor = weak_from_this();
		c->OnMount();
		c->SetEnabled(true);
	}

	void Actor::NotifyFunctionallyActiveStateChanged() {
		bool wasFA = functionallyActive;
		functionallyActive = (isRoot ? true : parentPtr.lock()->IsActive()) && active;
		if(wasFA == functionallyActive) return;
		for(auto child : children) {
			child->NotifyFunctionallyActiveStateChanged();
		}
		for(auto comp : components) {
			if(comp.second->enabled) {
				if(wasFA) {
					comp.second->OnDisable();
				} else {
					comp.second->OnEnable();
				}
			}
		}
	}

	void Actor::SetActive(bool state) {
		if(state == active) return;
		active = state;
		NotifyFunctionallyActiveStateChanged();
	}

	std::vector<ActorHandle> Actor::GetAllChildren() const {
		std::vector<ActorHandle> handles;
		for(std::shared_ptr<Actor> child : children) {
			ActorHandle& ah = handles.emplace_back();
			ah.actor = child;
			ah.world = world.lock();
		}
		return handles;
	}

	ActorHandle Actor::Create(const std::string& name, ActorHandle parent, xg::Guid guid) {
		Check<NonexistentValueException>(!parent.IsNull(), "Cannot make an actor with a null handle for a parent!");

		//Make actor
		ActorHandle hnd;
		hnd.actor = std::shared_ptr<Actor>(new Actor(name, parent, (guid == xg::Guid {} ? xg::newGuid() : guid)));
		hnd.world = parent.world;

		//Return actor
		return hnd;
	}

	ActorHandle Actor::Create(const std::string& name, std::shared_ptr<World> world, xg::Guid guid) {
		//Make actor
		ActorHandle hnd;
		hnd.actor = std::shared_ptr<Actor>(new Actor(name, world->root, (guid == xg::Guid {} ? xg::newGuid() : guid)));
		hnd.world = world;

		//Return actor
		return hnd;
	}

	Actor::Actor(const std::string& name, ActorHandle parent, xg::Guid guid)
	  : name(name), guid(guid), transform({0, 0, 0}, {0, 0, 0}, {1, 1, 1}), parentPtr(parent.actor), world(parent->world), active(true), functionallyActive(true) {
		NotifyFunctionallyActiveStateChanged();
	}
}