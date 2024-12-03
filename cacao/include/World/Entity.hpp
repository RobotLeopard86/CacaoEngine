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
#include <mutex>

//This is required for uint64_t used by crossguid (but that doesn't include it for some reason)
#include <stdint.h>

#include "crossguid/guid.hpp"
#include "glm/mat4x4.hpp"

namespace Cacao {
	//Forward declaration of Component so its header can include this one
	class Component;

	//Forward declaration of Entity for the fake deleter
	class Entity;

	/**
	 * @brief An object in the world
	 *
	 * @warning You @b MUST keep a reference to an entity as long as it is in a world, or you will cause a segmentation fault
	 */
	class Entity {
	  public:
		///@brief GUID (Globally Unique Identifier)
		const xg::Guid guid;

		/**
		 * @brief Check if this entity is active
		 *
		 * @return If the entity is active
		 */
		const bool IsActive() {
			return active;
		}

		/**
		 * @brief Activate or deactivate this entity
		 *
		 * @param val The new activation state
		 */
		void SetActive(bool val) {
			active = val;
		}

		/**
		 * @brief Get the local-space transform
		 * @details This transform is relative to the entity's parent
		 *
		 * @return A mutable reference to the local-space transform
		 */
		Transform& GetLocalTransform() {
			return transform;
		}

		/**
		 * @brief Get the world-space transformation matrix
		 *
		 * @return A matrix representing all transformations on this entity
		 */
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

		/**
		 * @brief Get the list of child entities
		 *
		 * @return A copy of the child entity list
		 */
		std::vector<std::shared_ptr<Entity>> GetChildrenAsList() {
			return children;
		}

		///@brief Human-readable name
		std::string name;

		/**
		 * @brief Create a new entity
		 *
		 * @param name The name of this entity
		 */
		Entity(std::string name)
		  : guid(xg::newGuid()), name(name), transform(glm::vec3 {0}, glm::vec3 {0}, glm::vec3 {1}), self(this, FakeDeleter<Entity> {}), parent(self), active(true) {}


		/**
		 * @brief Add a component to this entity
		 *
		 * @details All arguments are forwarded to the constructor of the component type.
		 *
		 * @return The component's GUID (Needed to access the component)
		 *
		 * @note Components created here will continue to exist after the entity is destroyed if another object still holds a reference to it, but will not be accessible
		 */
		template<typename T, typename... Args>
		const xg::Guid MountComponent(Args&&... args) {
			static_assert(std::is_base_of<Component, T>(), "Can only mount subclasses of Component!");

			//Create a component and set its owner
			std::shared_ptr<T> cptr = std::make_shared<T>(std::forward<Args>(args)...);
			cptr->SetOwner(self);

			{
				std::lock_guard lk(componentsMtx);

				//Add this component to the list
				auto mapValue = components.emplace(std::make_pair<xg::Guid, std::shared_ptr<Component>>(xg::newGuid(), std::static_pointer_cast<Component>(cptr)));

				//Return the GUID
				return mapValue.first->first;
			}
		}

		/**
		 * @brief Get all components on the entity
		 *
		 * @return A copy of the components on this entity
		 */
		std::map<xg::Guid, std::shared_ptr<Component>> GetAllComponents() {
			return components;
		}

		/**
		 * @brief Access a component on this entity
		 *
		 * @param guid The GUID of the component (returned from MountComponent)
		 *
		 * @return The component
		 *
		 * @throws Exception If no component with the provided GUID exists or it could not be cast to the specified type
		 */
		template<typename T>
		std::shared_ptr<T> GetComponent(xg::Guid guid) {
			static_assert(std::is_base_of<Component, T>(), "Can only get subclasses of Component!");
			std::lock_guard lk(componentsMtx);
			CheckException(components.contains(guid), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "No component with the provided GUID exists in this entity!");
			;

			//Try to cast the value
			try {
				return std::dynamic_pointer_cast<T>(components[guid]);
			} catch(std::bad_cast) {
				CheckException(false, Exception::GetExceptionCodeFromMeaning("WrongType"), "The requested compnent's type does not match the requested type!");
				return std::shared_ptr<T>();
			}
		}

		/**
		 * @brief Remove a component from this entity
		 *
		 * @note If there are other references to the component, it will not be freed by this call
		 *
		 * @param guid The GUID of the component (returned from MountComponent)
		 *
		 * @throws Exception If no component with the provided GUID exists
		 */
		void DeleteComponent(xg::Guid guid) {
			std::lock_guard lk(componentsMtx);
			CheckException(components.contains(guid), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "No component with the provided GUID exists in this entity!");
			;

			components[guid].reset();
			components.erase(guid);
		}

		/**
		 * @brief Set the parent of this entity
		 *
		 * @note To add an entity to a World, set its parent to the world's root entity
		 *
		 * @param newParent The new parent of the entity
		 */
		void SetParent(std::shared_ptr<Entity> newParent) {
			//If we aren't orphaned, remove ourselves from the current parent
			if(!parent.expired() && parent.lock() != self) {
				std::shared_ptr<Entity> parentLk = parent.lock();
				auto it = std::find(parentLk->children.begin(), parentLk->children.end(), self);
				if(it != parentLk->children.end()) parentLk->children.erase(it);
			}

			//Add ourselves as a child to the new parent
			newParent->children.push_back(self);

			//Set parent pointer
			parent = newParent;
		}

		/**
		 * @brief Destroy the entity and release references to components and child entities
		 */
		~Entity() {
			SetParent(self);

			//Release ownership of all components
			{
				std::lock_guard lk(componentsMtx);
				for(auto comp : components) {
					comp.second.reset();
				}
			}

			//Release ownership of all children
			for(auto child : children) {
				child.reset();
			}
		}

	  private:
		//Components on this entity
		std::map<xg::Guid, std::shared_ptr<Component>> components;
		std::mutex componentsMtx;

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

		friend class DynTickController;
	};
}