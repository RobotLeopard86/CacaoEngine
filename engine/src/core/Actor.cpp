#include "Cacao/Actor.hpp"
#include "Cacao/Component.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/World.hpp"

#include <algorithm>
#include <memory>

#define SELF_HANDLE [this]() {ActorHandle h; h.actor = shared_from_this(); h.world = world.lock(); return h; }()
#define SELF_HANDLE_NOCONST [this]() {ActorHandle h; h.actor = std::const_pointer_cast<Actor>(shared_from_this()); h.world = world.lock(); return h; }()

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
		ActorHandle current = SELF_HANDLE_NOCONST;
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
		auto it = std::find_if(parent->children.begin(), parent->children.end(), [&selfPtr](ActorHandle a) {
			return a.actor == selfPtr;
		});
		if(it != parent->children.end()) parent->children.erase(it);

		//Make sure we aren't parenting to ourselves
		Check<BadValueException>(newParent.actor != selfPtr, "Cannot parent an Actor to itself!");

		//Add ourselves as a child to the new parent
		newParent->children.push_back(SELF_HANDLE);

		//Set parent pointer
		parentPtr = newParent.actor;
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

	ActorHandle Actor::Create(const std::string& name, ActorHandle parent) {
		Check<NonexistentValueException>(!parent.IsNull(), "Cannot make an actor with a null handle for a parent!");

		//Make actor
		ActorHandle hnd;
		hnd.actor = std::shared_ptr<Actor>(new Actor(name, parent));
		hnd.world = parent.world;

		//Return actor
		return hnd;
	}

	ActorHandle Actor::Create(const std::string& name, std::shared_ptr<World> world) {
		//Make actor
		ActorHandle hnd;
		hnd.actor = std::shared_ptr<Actor>(new Actor(name, world->root));
		hnd.world = world;

		//Return actor
		return hnd;
	}

	Actor::Actor(const std::string& name, ActorHandle parent)
	  : name(name), guid(xg::newGuid()), transform({0, 0, 0}, {0, 0, 0}, {1, 1, 1}), parentPtr(parent.actor), world(parent->world), active(true), functionallyActive(true) {
		NotifyFunctionallyActiveStateChanged();
	}
}