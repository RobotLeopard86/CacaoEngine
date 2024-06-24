#pragma once

#include <vector>
#include <optional>
#include <random>

#include "3D/Mesh.hpp"
#include "Graphics/Material.hpp"
#include "3D/Transform.hpp"
#include "Core/Log.hpp"
#include "Core/Exception.hpp"

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "uuid_v4.h"
#include "glm/mat4x4.hpp"

namespace Cacao {
	//Forward declaration of Component so its header can include this one
	class Component;

	//Forward declaration of Entity for the fake deleter
	class Entity;

	//Fake deleter that doesn't actually delete anything
	//This is used on the self pointer so it doesn't try to delete us in the destructor
	struct FakeDeleter {
		void operator()(Entity* e) const {}
	};

	//An object in the world
	class Entity {
	  public:
		//UUID
		const UUIDv4::UUID uuid;

		const bool IsActive() {
			return active;
		}
		void SetActive(bool val) {
			active = val;
		}

		//Local space transform (relative to parent)
		Transform& GetLocalTransform() {
			return transform;
		}

		//World space transformation matrix
		//Changes made will not reflect on the entity
		glm::mat4 GetWorldTransformMatrix() {
			//Calculate the transformation matrix
			std::shared_ptr<Entity> curParent = parent.lock();
			glm::mat4 transMat = transform.GetTransformationMatrix();
			while(true) {
				//If the next parent's parent is itself (so orphaned or world root), we stop
				if(curParent->parent.lock() == curParent) break;

				//Apply this transformation
				transMat = curParent->GetLocalTransform().GetTransformationMatrix() * transMat;

				//Otherwise, move on to the next parent
				curParent = curParent->parent.lock();
			}

			return transMat;
		}

		//Get a copy of the child entity list
		std::vector<std::shared_ptr<Entity>> GetChildrenAsList() {
			return children;
		}

		//Human-readable name
		std::string name;

		Entity(std::string name)
		  : uuid(UUIDv4::UUIDGenerator<std::mt19937_64>().getUUID()), name(name), transform(glm::vec3 {0}, glm::vec3 {0}, glm::vec3 {1}), self(this, FakeDeleter {}), parent(self), active(true) {}


		//Add a component to this entity
		//Functions like a constructor
		//Returns a UUID for accessing this component (DO NOT LOSE THIS)
		//All components will be removed and deleted when this object is deleted
		//Component freeing will only occur once all holders release ownership, but it will no longer be accessible through the entity
		template<typename T, typename... Args>
		const UUIDv4::UUID MountComponent(Args&&... args) {
			static_assert(std::is_base_of<Component, T>(), "Can only mount subclasses of Component!");

			//Create a component and set its owner
			std::shared_ptr<T> cptr = std::make_shared<T>(std::forward<Args>(args)...);
			cptr->SetOwner(self);

			//Add this component to the list
			auto mapValue = components.emplace(std::make_pair<UUIDv4::UUID, std::shared_ptr<Component>>(UUIDv4::UUIDGenerator<std::mt19937_64>().getUUID(), std::static_pointer_cast<Component>(cptr)));

			//Return the UUID
			return mapValue.first->first;
		}

		//Retrieve a component from the entity
		//Requires the UUID returned from MountComponent
		template<typename T>
		std::shared_ptr<T> GetComponent(UUIDv4::UUID uuid) {
			static_assert(std::is_base_of<Component, T>(), "Can only get subclasses of Component!");
			CheckException(components.contains(uuid), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "No component with the provided UUID exists in this entity!");

			//Try to cast the value
			try {
				return std::dynamic_pointer_cast<T>(components[uuid]);
			} catch(std::bad_cast) {
				CheckException(false, Exception::GetExceptionCodeFromMeaning("WrongType"), "The requested compnent's type does not match the requested type!")
			}
		}

		//Remove a component from the entity and delete it
		//The freeing will only occur once all holders release ownership, but it will no longer be accessible through the entity
		//Requires the UUID returned from MountComponent
		void DeleteComponent(UUIDv4::UUID uuid) {
			CheckException(components.contains(uuid), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "No component with the provided UUID exists in this entity!");

			components[uuid].reset();
			components.erase(uuid);
		}

		//Get the components in list form
		std::vector<std::shared_ptr<Component>> GetComponentsAsList() {
			std::vector<std::shared_ptr<Component>> out;
			for(auto comp : components) {
				out.push_back(comp.second);
			}
			return out;
		}

		//Change parent to another entity
		void SetParent(std::shared_ptr<Entity> newParent) {
			//If we aren't orphaned, remove ourselves from the current parent
			if(parent.lock() != self) {
				std::shared_ptr<Entity> parentLk = parent.lock();
				auto it = std::find(parentLk->children.begin(), parentLk->children.end(), self);
				if(it != parentLk->children.end()) parentLk->children.erase(it);
			}

			//Add ourselves as a child to the new parent
			newParent->children.push_back(self);

			//Set parent pointer
			parent = newParent;
		}

		~Entity() {
			//Release ownership of all components
			for(auto comp : components) {
				comp.second.reset();
			}
			//Release ownership of all children
			for(auto child : children) {
				child.reset();
			}
		}

	  private:
		//Components on this entity
		std::map<UUIDv4::UUID, std::shared_ptr<Component>> components;

		//Child entities
		std::vector<std::shared_ptr<Entity>> children;

		//Transform
		Transform transform;

		//Shared pointer to self (used for component ownership)
		std::shared_ptr<Entity> self;

		//Weak pointer to parent
		std::weak_ptr<Entity> parent;

		//Is this entity active?
		bool active;
	};
}