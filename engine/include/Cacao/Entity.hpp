#pragma once

#include "DllHelper.hpp"
#include "Transform.hpp"
#include "Exceptions.hpp"
#include "Event.hpp"

#include <typeindex>
#include <map>
#include <memory>
#include <optional>

#include "crossguid/guid.hpp"

namespace Cacao {
	class Component;

	class CACAO_API Entity {
	  public:
		/**
		 * @brief Create a new entity
		 *
		 * @note You can't use the constructor because the entity needs to have an accesible shared_ptr to itself
		 *
		 * @param name The initial name of the entity
		 * @param parent The initial entity parent (set this to nullopt to create an orphaned entity)
		 *
		 * @return A shared_ptr to the entity
		 */
		static std::shared_ptr<Entity> Create(const std::string& name, std::optional<std::shared_ptr<Entity>> parent);

		std::string name;	///<The human-readable name of the entity
		xg::Guid guid;		///<Entity ID, unique
		Transform transform;///<Entity transform relative to parent

		/**
		 * @brief Calculate the world-space transformation matrix of the entity
		 *
		 * @return The world-space transformation matrix
		 */
		glm::mat4 GetWorldTransformMatrix();

		/**
		 * @brief Access the parent of this entity
		 *
		 * @return The parent entity, or an empty pointer if this is an orphaned entity
		 */
		std::shared_ptr<Entity> GetParent() {
			return parentPtr;
		}

		/**
		 * @brief Check if the entity is active
		 *
		 * @return If the entity is active
		 */
		bool IsActive() {
			return active;
		}

		/**
		 * @brief Activate or deactivate the entity
		 *
		 * @param state The new activation state
		 */
		void SetActive(bool state);

		/**
		 * @brief Change the parent of this entity
		 *
		 * @param newParent The new parent of this entity, or nullopt to orphan the entity
		 */
		void Reparent(std::optional<std::shared_ptr<Entity>> newParent);

		/**
		 * @brief Create a new component and add it to this entity
		 *
		 * @param args The arguments to the component constructor
		 *
		 * @throws ContainerException If a component of this type already exists on the entity
		 */
		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T> && std::is_constructible_v<T, Args&&...>
		void MountComponent(Args&&... args) {
			Check<ContainerException>(!components.contains(std::type_index(typeid(T))), "A component of the type specified already exists on the entity!");
			components.insert_or_assign(std::type_index(typeid(T)), std::make_shared<T>(std::forward<Args...>(args)));
		}

		/**
		 * @brief Check if a component is on an entity
		 *
		 * @return Whether a component of the type is on the entity
		 */
		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T>
		bool HasComponent() {
			return components.contains(std::type_index(typeid(T)));
		}

		/**
		 * @brief Access a component on the entity
		 *
		 * @return The component
		 *
		 * @throws ContainerException If a component of this type does not exist on the entity
		 */
		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T>
		std::shared_ptr<T> GetComponent() {
			Check<ContainerException>(components.contains(std::type_index(typeid(T))), "A component of the type specified does not exist on the entity!");
			return std::dynamic_pointer_cast<T>(components.at(std::type_index(typeid(T))));
		}

		/**
		 * @brief Delete a component from the entity
		 *
		 * @throws ContainerException If a component of this type does not exist on the entity
		 */
		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T>
		void GetComponent() {
			Check<ContainerException>(components.contains(std::type_index(typeid(T))), "A component of the type specified does not exist on the entity!");
			components.erase(std::type_index(typeid(T)));
		}

		/**
		 * @brief Get a copy of all the components on the entity
		 *
		 * @note This doesn't actually copy the components, just their pointers, but this does increment the reference count
		 *
		 * @return All entity components
		 */
		std::map<std::type_index, std::shared_ptr<Component>> GetAllComponents() {
			return components;
		}

		/**
		 * @brief Get all the children of the entity
		 *
		 * @return All child entities
		 */
		std::vector<std::shared_ptr<Entity>> GetAllChildren() {
			return children;
		}

		/**
		 * @brief Direction of entity message travel
		 */
		enum class MessageSendDirection {
			Down,	 ///<Send the message to children
			Up,		 ///<Send the message to parents
			Broadcast///<Send the message to all entities connected to this one
		};

		/**
		 * @brief Send a message to connected entities
		 *
		 * @param msg An Event or DataEvent to send
		 * @param dir The sending direction
		 * @param recurse Whether or not to recurse through connected entities of recipients (ignored if dir is Broadcast)
		 */
		void SendMessage(Event& msg, MessageSendDirection dir, bool recurse);

	  private:
		Entity(const std::string& name, std::optional<std::shared_ptr<Entity>> parent);
		~Entity();

		std::shared_ptr<Entity> selfPtr, parentPtr;
		std::map<std::type_index, std::shared_ptr<Component>> components;
		std::vector<std::shared_ptr<Entity>> children;

		void MessageReceiver(Event& msg, MessageSendDirection dir, bool recurse, std::shared_ptr<Entity> originator);
		void HandleMessage(Event& msg);
		void PassOnMessage(Event& msg, MessageSendDirection dir, bool recurse, std::shared_ptr<Entity> originator);

		bool active, wasActive;
	};
}