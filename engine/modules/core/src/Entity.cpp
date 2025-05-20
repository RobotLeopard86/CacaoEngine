#include "Cacao/Entity.hpp"

namespace Cacao {
	glm::mat4 Entity::GetWorldTransformMatrix() {
		//Calculate the transformation matrix
		//This should take a (0, 0, 0) coordinate relative to the entity and turn it into a world space transform
		std::shared_ptr<Entity> curParent = parentPtr;
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

	void Entity::Reparent(std::optional<std::shared_ptr<Entity>> newParent) {
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
		std::shared_ptr<Entity> np = newParent.value();
		if(np == selfPtr) {
			parentPtr = selfPtr;
			return;
		}

		//Add ourselves as a child to the new parent
		np->children.push_back(selfPtr);

		//Set parent pointer
		parentPtr = np;
	}

	Entity::~Entity() {
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

	void Entity::MessageReceiver(Event& msg, MessageSendDirection dir, bool recurse, std::shared_ptr<Entity> originator) {
		//Handle the message for ourselves
		HandleMessage(msg);

		//If we should pass this on, do it
		PassOnMessage(msg, dir, recurse, originator);
	}

	void Entity::PassOnMessage(Event& msg, MessageSendDirection dir, bool recurse, std::shared_ptr<Entity> originator) {
		if(dir != MessageSendDirection::Broadcast) {
			if(recurse) {
				if(dir == MessageSendDirection::Up) {
					parentPtr->MessageReceiver(msg, dir, recurse, originator);
				} else {
					for(auto child : children) {
						child->MessageReceiver(msg, dir, recurse, originator);
					}
				}
			}
		} else {
			//Send the message down to all children except the originator
			for(auto child : children) {
				if(child == originator) continue;
				child->MessageReceiver(msg, MessageSendDirection::Down, recurse, originator);
			}

			//Send the message up to the parent
			parentPtr->MessageReceiver(msg, MessageSendDirection::Up, recurse, selfPtr);
		}
	}

	void Entity::SendMessage(Event& msg, MessageSendDirection dir, bool recurse) {
		//Start the pass-on chain
		PassOnMessage(msg, dir, recurse, selfPtr);
	}

	std::shared_ptr<Entity> Entity::Create(const std::string& name, std::optional<std::shared_ptr<Entity>> parent) {
		//Make entity
		std::shared_ptr<Entity> e = std::make_shared<Entity>(name, parent);

		//Give entity its self pointer
		e->selfPtr = e;

		//Set entity's parent to itself if requested
		if(!parent.has_value()) e->parentPtr = e->selfPtr;

		//Return entity
		return e;
	}

	Entity::Entity(const std::string& name, std::optional<std::shared_ptr<Entity>> parent)
	  : name(name), guid(xg::newGuid()), transform({0, 0, 0}, {0, 0, 0}, {1, 1, 1}), active(true) {
		if(parent.has_value()) parentPtr = parent.value();
	}

	//BELOW HERE IS UNFINISHED!!!!

	void Entity::HandleMessage(Event& msg) {
		if(msg.GetType().compare("EntMsg_ActiveStateNotify")) {
			DataEvent<bool>& amsg = static_cast<DataEvent<bool>&>(msg);
			if(amsg.GetData()) {
				active = wasActive;
			} else {
				wasActive = active;
				active = false;
			}
		}
	}

	void Entity::SetActive(bool state) {
		//Set this entity's state
		active = state;
	}
}