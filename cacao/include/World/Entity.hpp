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

namespace Cacao {
	//Forward declaration of Component so its header can include this one
	class Component;

	//An object in the world
	class Entity {
	  public:
		//UUID
		const UUIDv4::UUID uuid;

		Transform transform;

		//Human-readable name
		std::string name;

		Entity(std::string name)
		  : uuid(UUIDv4::UUIDGenerator<std::mt19937_64>().getUUID()), transform(glm::vec3 {0}, glm::vec3 {0}, glm::vec3 {1}), name(name), active(true), self(this) {}

		//Is this entity active?
		bool active;

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

		//Child entities
		std::vector<std::shared_ptr<Entity>> children;

	  private:
		//Components on this entity
		std::map<UUIDv4::UUID, std::shared_ptr<Component>> components;

		//Shared pointer to self (used for component ownership)
		std::shared_ptr<Entity> self;
	};
}