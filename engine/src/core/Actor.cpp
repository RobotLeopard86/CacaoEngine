#include "Cacao/Actor.hpp"
#include "Cacao/CodeRegistry.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/World.hpp"
#include "crossguid/guid.hpp"

#include <algorithm>
#include <memory>

namespace Cacao {
	glm::mat4 Actor::GetWorldTransformMatrix() const {
		//Calculate the transformation matrix
		//This should take a (0, 0, 0) coordinate relative to the actor and turn it into a world space transform
		ActorRef current = parent;
		glm::mat4 transMat = transform.GetTransformationMatrix();
		do {
			//Apply transformation
			transMat = current->transform.GetTransformationMatrix() * transMat;
		} while(!(current = current->GetParent()));

		return transMat;
	}

	void Actor::Reparent(ActorRef newParent) {
		//TODO: Update to new system

		/*//Remove ourselves from the current parent
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
		parentPtr = newParent.actor;*/
	}

	void Actor::MountComponent(const std::string& factoryID) {
		//Try to create the object
		auto [ptr, type] = CodeRegistry::Get().Instantiate<Component>(factoryID);
		Check<ExistingValueException>(!components.contains(type), "A component of the type specified already exists on the actor!");

		//Call down to common component setup
		std::unique_ptr<Component> uptr(ptr);
		ComponentSetup(type, std::move(uptr));
	}

	void Actor::SetActive(bool state) {
		if(state == active) return;
		active = state;
	}

	void Actor::ComponentSetup(std::type_index type, std::unique_ptr<Component>&& ptr) {
		//Get or create new slot
		ComponentSlot& slot = components[type];

		//Update generation number to invalidate old references
		++(slot.generation);

		//Store component pointer
		slot.component = std::move(ptr);

		//Set up component object
		slot.component->owner = self;
		slot.component->OnMount();
		slot.component->SetEnabled(true);
	}

	Actor::Actor(const std::string& name, ActorRef parent, xg::Guid guid)
	  : name(name), guid(guid), transform({0, 0, 0}, {0, 0, 0}, {1, 1, 1}), parent(parent), world(parent->world), active(true) {}
}