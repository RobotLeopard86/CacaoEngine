#pragma once

#include <vector>
#include <optional>
#include <random>

#include "3D/Mesh.hpp"
#include "Graphics/Material.hpp"
#include "3D/Transform.hpp"
#include "Core/Log.hpp"
#include "Core/Exception.hpp"
#include "Utilities/MiscUtils.hpp"

#include <memory>
#include <string>
#include <map>
#include <vector>

//This is required for uint64_t used by crossguid (but that doesn't include it for some reason)
#include <stdint.h>

#include "crossguid/guid.hpp"
#include "glm/mat4x4.hpp"

namespace Cacao {
	//Forward declaration of Component so its header can include this one
	class Component;

	//Forward declaration of Entity for the fake deleter
	class Entity;

	//An object in the world
	class Entity {
	  public:
		//GUID
		const xg::Guid guid;

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
		  : guid(xg::newGuid()), name(name), transform(glm::vec3 {0}, glm::vec3 {0}, glm::vec3 {1}), self(this, FakeDeleter<Entity> {}), parent(self), active(true) {}


		//Add a component to this entity
		//Functions like a constructor
		//Returns a GUID for accessing this component (DO NOT LOSE THIS)
		//All components will be removed and deleted when this object is deleted
		//Component freeing will only occur once all holders release ownership, but it will no longer be accessible through the entity
		template<typename T, typename... Args>
		const xg::Guid MountComponent(Args&&... args) {
			static_assert(std::is_base_of<Component, T>(), "Can only mount subclasses of Component!");

			//Create a component and set its owner
			std::shared_ptr<T> cptr = std::make_shared<T>(std::forward<Args>(args)...);
			cptr->SetOwner(self);

			//Add this component to the list
			auto mapValue = components.emplace(std::make_pair<xg::Guid, std::shared_ptr<Component>>(xg::newGuid(), std::static_pointer_cast<Component>(cptr)));

			//Return the GUID
			return mapValue.first->first;
		}

		//Retrieve a component from the entity
		//Requires the GUID returned from MountComponent
		template<typename T>
		std::shared_ptr<T> GetComponent(xg::Guid guid) {
			static_assert(std::is_base_of<Component, T>(), "Can only get subclasses of Component!");
			CheckException(components.contains(guid), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "No component with the provided GUID exists in this entity!");

			//Try to cast the value
			try {
				return std::dynamic_pointer_cast<T>(components[guid]);
			} catch(std::bad_cast) {
				CheckException(false, Exception::GetExceptionCodeFromMeaning("WrongType"), "The requested compnent's type does not match the requested type!")
			}
		}

		//Remove a component from the entity and delete it
		//The freeing will only occur once all holders release ownership, but it will no longer be accessible through the entity
		//Requires the GUID returned from MountComponent
		void DeleteComponent(xg::Guid guid) {
			CheckException(components.contains(guid), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "No component with the provided GUID exists in this entity!");

			components[guid].reset();
			components.erase(guid);
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
		std::map<xg::Guid, std::shared_ptr<Component>> components;

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