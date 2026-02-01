#pragma once

#include "DllHelper.hpp"
#include "Transform.hpp"
#include "Exceptions.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>

#include "crossguid/guid.hpp"

namespace Cacao {
	class Actor;
	class World;

	/**
	 * @brief An handle for Actors to control world tree ownership
	 */
	class ActorRef {
	  public:
		/**
		 * @brief Create a new "null" ActorRef that is invalid
		 */
		ActorRef() {}

		/**
		 * @brief Access the underlying Actor
		 */
		Actor* operator->();

		/**
		 * @brief Access the underlying Actor, but constant
		 */
		const Actor* operator->() const;

		/**
		 * @brief Check if this handle is valid
		 *
		 * @return Whether this handle is managing a living Actor or not (null handle or destroyed actor)
		 */
		operator bool() const noexcept;

		/**
		 * @brief Check if two handles are equal (that is, they reference the same Actor)
		 *
		 * @return If the handles are equal
		 */
		bool operator==(const ActorRef& rhs) const noexcept;

	  private:
		friend class Actor;
		friend class World;

		//Non-owning World pointer
		World* world = nullptr;

		//Actor slot access information
		uint64_t slotID;
		uint64_t generation;

		//Hidden valid handle constructor
		ActorRef(World* world, uint64_t slot, uint64_t generation) : world(world), slotID(slot), generation(generation) {}
	};

	/**
	 * @brief An object attached to an Actor that performs tasks on its behalf
	 */
	class CACAO_API Component {
	  public:
		/**
		 * @brief Check if the component is enabled
		 * @details This takes into account both if the component itself is enabled and its owning Actor is active
		 *
		 * @note This will return false if the owning Actor is inactive
		 */
		bool IsEnabled() const;

		/**
		 * @brief Activate or deactivate the component
		 *
		 * @param state The new activation state
		 */
		void SetEnabled(bool state) {
			enabled = state;
			if(IsEnabled()) {
				OnEnable();
			}
		}

		/**
		 * @brief Get the owning actor of this component
		 *
		 * @return A reference to the owning actor
		 *
		 * @throws NonexistentValueException If the actor no longer exists
		 */
		ActorRef GetOwner() const {
			Check<NonexistentValueException>((bool)owner, "Cannot access the invalid owner of a component!");
			return owner;
		}

		virtual ~Component() {}

	  protected:
		Component();

		/**
		 * @brief Runs when the component is first mounted on an Actor
		 *
		 * @note All setup should be performed here, @b NOT in the constructor. Only when this function is called is the component properly configured.
		 */
		virtual void OnMount() {};

		/**
		 * @brief Runs when the component is deleted from an Actor
		 *
		 * @note All teardown should be performed here, @b NOT in the destructor. After this function is called, the component may be in an invalid state.
		 */
		virtual void OnDelete() {};

		/**
		 * @brief Runs when the component is enabled or when the owning Actor becomes active if the component was already enabled
		 */
		virtual void OnEnable() {};

		/**
		 * @brief Runs when the component is disabled or when the owning Actor becomes inactive if the component was already enabled
		 */
		virtual void OnDisable() {};

		friend class Actor;

	  private:
		bool enabled;
		ActorRef owner;
	};

	/**
	 * @brief An object that exists within a World
	 */
	class CACAO_API Actor {
	  public:
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
		 * @return The parent actor, or a null handle if this actor is toplevel
		 */
		ActorRef GetParent() const {
			return parent;
		}

		/**
		 * @brief Check if the actor is active
		 * @details This takes into account both if the actor is enabled and its parent Actor is active
		 *
		 * @note This will return false if the parent Actor is inactive
		 */
		bool IsActive() const;

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
		void Reparent(ActorRef newParent);

		/**
		 * @brief Check if a component is on an actor
		 *
		 * @return Whether a component of the type is on the actor
		 */
		template<typename T>
			requires std::is_base_of_v<Component, T>
		bool HasComponent() const {
			return components.contains(std::type_index(typeid(T))) && components.at(std::type_index(typeid(T))).component;
		}

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
			Check<ExistingValueException>(!HasComponent<T>(), "A component of the type specified already exists on the actor!");

			//Prepare objects
			std::type_index type(typeid(T));
			std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args...>(args...));

			//Call-down to internal function
			ComponentSetup(type, std::move(component));
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
		 * @brief Access a component on the actor
		 *
		 * @return The component
		 *
		 * @throws NonexistentValueException If a component of this type does not exist on the actor
		 */
		template<typename T>
			requires std::is_base_of_v<Component, T>
		std::shared_ptr<T> GetComponent() const {
			Check<NonexistentValueException>(HasComponent<T>(), "A component of the type specified does not exist on the actor!");
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
			Check<NonexistentValueException>(HasComponent<T>(), "A component of the type specified does not exist on the actor!");
			components.erase(std::type_index(typeid(T)));
		}

		/**
		 * @brief Get a copy of all the components on the actor
		 *
		 * @note This doesn't actually copy the components, just their pointers, but this does increment the reference count
		 *
		 * @return All actor components
		 */
		std::unordered_map<std::type_index, std::unique_ptr<Component>&> GetAllComponents();

		/**
		 * @brief Get all the children of the actor
		 *
		 * @return All child entities
		 */
		std::vector<ActorRef> GetAllChildren() const {
			return children;
		}

	  private:
		Actor(const std::string& name, ActorRef parent, xg::Guid);
		friend class World;
		friend class Component;

		struct ComponentSlot {
			std::unique_ptr<Component> component;
			uint64_t generation = 1;
		};

		ActorRef parent;
		std::vector<ActorRef> children;
		std::unordered_map<std::type_index, ComponentSlot> components;
		World* world;

		void ComponentSetup(std::type_index type, std::unique_ptr<Component>&& ptr);

		bool active;
	};
}