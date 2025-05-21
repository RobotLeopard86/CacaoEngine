#include "Cacao/Actor.hpp"

#include <algorithm>

namespace Cacao {
	glm::mat4 Actor::GetWorldTransformMatrix() {
		//Calculate the transformation matrix
		//This should take a (0, 0, 0) coordinate relative to the actor and turn it into a world space transform
		std::shared_ptr<Actor> curParent = parentPtr;
		glm::mat4 transMat = transform.GetTransformationMatrix();
		while(true) {
			//If the next parent's parent is itself (so orphaned or world root), we stop
			if(curParent->parentPtr == curParent) break;

			//Apply this transformation
			transMat = curParent->transform.GetTransformationMatrix() * transMat;

			//Otherwise, move on to the next parent
			curParent = curParent->parentPtr;
		}

		return transMat;
	}

	void Actor::Reparent(std::optional<std::shared_ptr<Actor>> newParent) {
		//If we aren't orphaned, remove ourselves from the current parent
		if(!parentPtr || (parentPtr && parentPtr != selfPtr)) {
			auto it = std::find(parentPtr->children.begin(), parentPtr->children.end(), selfPtr);
			if(it != parentPtr->children.end()) parentPtr->children.erase(it);
		}

		//Check if we want to become orphaned
		if(!newParent.has_value()) {
			parentPtr = selfPtr;
			return;
		}

		//Get parent value
		std::shared_ptr<Actor> np = newParent.value();
		if(np == selfPtr) {
			parentPtr = selfPtr;
			return;
		}

		//Add ourselves as a child to the new parent
		np->children.push_back(selfPtr);

		//Set parent pointer
		parentPtr = np;
	}

	Actor::~Actor() {
		//Become an orphan
		Reparent(std::nullopt);

		//Release ownership of all components
		for(auto comp : components) {
			comp.second.reset();
		}

		//Release ownership of all children
		for(auto child : children) {
			child.reset();
		}
	}

	std::shared_ptr<Actor> Actor::Create(const std::string& name, std::optional<std::shared_ptr<Actor>> parent) {
		//Make actor
		std::shared_ptr<Actor> e = std::shared_ptr<Actor>(new Actor(name, parent));

		//Give actor its self pointer
		e->selfPtr = e;

		//Set actor's parent to itself if requested
		if(!parent.has_value()) e->parentPtr = e->selfPtr;

		//Return actor
		return e;
	}

	Actor::Actor(const std::string& name, std::optional<std::shared_ptr<Actor>> parent)
	  : name(name), guid(xg::newGuid()), transform({0, 0, 0}, {0, 0, 0}, {1, 1, 1}), active(true) {
		if(parent.has_value()) parentPtr = parent.value();
	}
}