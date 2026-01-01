#pragma once

#include "DllHelper.hpp"
#include "Transform.hpp"
#include "Exceptions.hpp"

#include <unordered_map>
#include <memory>
#include <typeindex>

#include "crossguid/guid.hpp"

namespace Cacao {
	class Component;
	class Actor;
	class World;

	/**
	 * @brief An ownership-management handle for Actors to maintain the actor tree
	 */
	class ActorHandle {
	  public:
		/**
		 * @brief Create a new "null" ActorHandle pointing to nothing
		 */
		ActorHandle() {}

		/**
		 * @brief Access the underlying Actor
		 */
		Actor* operator->() {
			return actor.get();
		}

		/**
		 * @brief Access the underlying Actor, but constant
		 */
		const Actor* operator->() const {
			return actor.get();
		}

		/**
		 * @brief Check if this is a "null" handle
		 *
		 * @return Whether this handle is managing an Actor or not
		 */
		bool IsNull() {
			return (bool)actor;
		}

	  private:
		friend class Actor;
		friend class World;

		//Owning reference to actor
		std::shared_ptr<Actor> actor;

		//Owning reference to actor world
		std::shared_ptr<World> world;
	};

	/**
	 * @brief An object that exists within a World
	 */
	class CACAO_API Actor : public std::enable_shared_from_this<Actor> {
	  public:
		/**
		 * @brief Create a new actor
		 *
		 * @param name The initial name of the actor
		 * @param parent The initial actor parent
		 * @param guid An optional GUID to provide the actor with
		 *
		 * @return A handle to the new actor
		 *
		 * @throws NonexistentValueException If the provided parent is a null handle
		 */
		static ActorHandle Create(const std::string& name, ActorHandle parent, xg::Guid guid = {});

		/**
		 * @brief Create a new actor attached to the world root
		 *
		 * @param name The initial name of the actor
		 * @param world The world to place the actor in
		 *
		 * @return A handle to the new actor
		 */
		static ActorHandle Create(const std::string& name, std::shared_ptr<World> world, xg::Guid guid = {});

		std::string name;	///<The human-readable name of the actor
		const xg::Guid guid;///<Actor ID, unique
		Transform transform;///<Actor transform relative to parent

		/**
		 * @brief Calculate the world-space transformation matrix of the actor
		 *
		 * @return The world-space transformation matrix
		 */
		glm::mat4 GetWorldTransformMatrix() const;

		/**
		 * @brief Access the parent of this actor
		 *
		 * @return The parent actor, or a null handle if the parent is the world root
		 */
		ActorHandle GetParent() const;

		/**
		 * @brief Check if the actor is active
		 * @details This takes into account both if the actor is enabled and its parent Actor is active
		 *
		 * @note This will return false if the parent Actor is inactive
		 */
		bool IsActive() const {
			return functionallyActive;
		}

		/**
		 * @brief Activate or deactivate the actor
		 *
		 * @param state The new activation state
		 */
		void SetActive(bool state);

		/**
		 * @brief Change the parent of this actor
		 *
		 * @param newParent The new parent of this actor
		 */
		void Reparent(ActorHandle newParent);

		/**
		 * @brief Create a new component and add it to this actor
		 *
		 * @param args The arguments to the component constructor
		 *
		 * @throws ExistingValueException If a component of this type already exists on the actor
		 */
		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T> && std::is_constructible_v<T, Args&&...>
		void MountComponent(Args&&... args) {
			Check<ExistingValueException>(!components.contains(std::type_index(typeid(T))), "A component of the type specified already exists on the actor!");
			components.insert_or_assign(std::type_index(typeid(T)), std::make_shared<T>(std::forward<Args...>(args...)));
			PostMountComponent(components[std::type_index(typeid(T))]);
		}

		/**
		 * @brief Create a new component and add it to this actor
		 *
		 * @param factoryID The ID of the Component factory in with the CodeRegistry to create the component
		 *
		 * @throws ExistingValueException If a component of this type already exists on the actor
		 * @throws NonexistentValueException If the CodeRegistry does not have a Component actory registered for the provided ID
		 */
		void MountComponent(const std::string& factoryID);

		/**
		 * @brief Check if a component is on an actor
		 *
		 * @return Whether a component of the type is on the actor
		 */
		template<typename T>
			requires std::is_base_of_v<Component, T>
		bool HasComponent() const {
			return components.contains(std::type_index(typeid(T)));
		}

		/**
		 * @brief Access a component on the actor
		 *
		 * @return The component
		 *
		 * @throws NonexistentValueException If a component of this type does not exist on the actor
		 */
		template<typename T>
			requires std::is_base_of_v<Component, T>
		std::shared_ptr<T> GetComponent() const {
			Check<NonexistentValueException>(components.contains(std::type_index(typeid(T))), "A component of the type specified does not exist on the actor!");
			return std::dynamic_pointer_cast<T>(components.at(std::type_index(typeid(T))));
		}

		/**
		 * @brief Delete a component from the actor
		 *
		 * @throws NonexistentValueException If a component of this type does not exist on the actor
		 */
		template<typename T>
			requires std::is_base_of_v<Component, T>
		void DeleteComponent() {
			Check<NonexistentValueException>(components.contains(std::type_index(typeid(T))), "A component of the type specified does not exist on the actor!");
			components.erase(std::type_index(typeid(T)));
		}

		/**
		 * @brief Get a copy of all the components on the actor
		 *
		 * @note This doesn't actually copy the components, just their pointers, but this does increment the reference count
		 *
		 * @return All actor components
		 */
		std::unordered_map<std::type_index, std::shared_ptr<Component>> GetAllComponents() const {
			return components;
		}

		/**
		 * @brief Get all the children of the actor
		 *
		 * @return All child entities
		 */
		std::vector<ActorHandle> GetAllChildren() const;

		~Actor();

	  private:
		Actor(const std::string& name, ActorHandle parent, xg::Guid);
		friend class World;

		std::weak_ptr<Actor> parentPtr;
		std::weak_ptr<World> world;
		std::unordered_map<std::type_index, std::shared_ptr<Component>> components;
		std::vector<std::shared_ptr<Actor>> children;

		void PostMountComponent(std::shared_ptr<Component> c);
		void NotifyFunctionallyActiveStateChanged();

		bool active, functionallyActive;
		bool isRoot = false;
	};
}