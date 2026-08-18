#pragma once

#include "DllHelper.hpp"
#include "Transform.hpp"
#include "Exceptions.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>

#include "astra/serialized_substitute.hpp"
#include "crossguid/guid.hpp"
#include "astra/setup.hpp"
#include "astra/type_actions/common_actions.hpp"

namespace Cacao {
	class Actor;
	class World;

	/**
	 * @brief An handle for Actors to control world tree ownership
	 */
	class CACAO_API ActorRef {
	  public:
		/**
		 * @brief Create a new "null" ActorRef that is invalid
		 */
		ActorRef() {}

		/**
		 * @brief Access the underlying Actor
		 *
		 * @return A non-owning pointer to the Actor object
		 *
		 * @throws NonexistentValueException If this handle is invalid
		 */
		Actor* operator->();

		/**
		 * @brief Access the underlying Actor constly
		 *
		 * @return A non-owning pointer to the Actor object
		 *
		 * @throws NonexistentValueException If this handle is invalid
		 */
		const Actor* operator->() const;

		/**
		 * @brief Access the World containing the underlying Actor
		 *
		 * @return A non-owning pointer to the World object
		 *
		 * @throws NonexistentValueException If this handle is invalid
		 */
		World* GetWorld();

		/**
		 * @brief Constly access the World containing the underlying Actor
		 *
		 * @return A non-owning pointer to the World object
		 *
		 * @throws NonexistentValueException If this handle is invalid
		 */
		const World* GetWorld() const;

		/**
		 * @brief Check if this handle is valid
		 *
		 * @return Whether this handle is managing a living Actor or not (null handle or destroyed actor)
		 */
		operator bool() const noexcept;

		/**
		 * @brief Check if two handles are equal (that is, they reference the same Actor or are both null)
		 *
		 * @return If the handles are equal
		 */
		bool operator==(const ActorRef& rhs) const noexcept;

	  private:
		friend class Actor;
		friend class World;
		friend astra::SerializedSubstitute<ActorRef>;

		//Non-owning World pointer
		std::weak_ptr<World> world;

		//Actor slot access information
		uint64_t slotID;
		uint64_t generation;

		//Null state
		bool null = true;

		//Hidden valid handle constructor
		ActorRef(std::weak_ptr<World> world, uint64_t slot, uint64_t generation);

		//Hidden resolver function
		void* Resolve() const noexcept;
	};

	/**
	 * @brief An object attached to an Actor that performs tasks on its behalf
	 */
	class CACAO_API ASTRA_REFLECT Component : public AstraReflectBase {
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

		ASTRASETUP(Component)

#ifdef _ASTRAGENERATE
		Component() {}
#endif
	  protected:
#ifndef _ASTRAGENERATE
		Component() {}
		friend struct astra::CommonActions<Component>;
#endif
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
		virtual void OnUnmount() {};

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
		ASTRA_IGNORE bool enabled;
		ASTRA_IGNORE ActorRef owner;
	};

	/**
	 * @brief An object that exists within a World
	 */
	class CACAO_API Actor {
	  public:
		std::string name;	///<The human-readable name of the actor
		const xg::Guid guid;///<Actor ID, unique

		/**
		 * @brief Get the actor's local transform relative to its parent
		 *
		 * @return The local-space transform
		 */
		const Transform& GetLocalTransform() const {
			return transform;
		}

		/**
		 * @brief Update the local transform of the actor relative to its parent
		 *
		 * @param transform The new local-space transform
		 */
		void SetLocalTransform(Transform transform) {
			transformDirty = true;
			this->transform = transform;
		}

		/**
		 * @brief Calculate the world-space transformation matrix of the actor
		 *
		 * @return The world-space transformation matrix
		 */
		glm::mat4 GetWorldTransformationMatrix() const;

		/**
		 * @brief Get the effective transform of the actor in world-space
		 *
		 * @return The world-space transform
		 */
		Transform GetWorldTransform() const;

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
		 *
		 * @throws BadValueException If a null ref is provided
		 * @throws BadValueException If a ref to the same actor being reparented is provided
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
		T& MountComponent(Args&&... args, bool enabled = true) {
			Check<ExistingValueException>(!HasComponent<T>(), "A component of the type specified already exists on the actor!");

			//Prepare objects
			std::type_index type(typeid(T));
			std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args...>(args...));

			//Call-down to internal function
			_ComponentSetup(type, std::move(component), enabled);

			//Return the component reference
			return static_cast<T&>(*components[type].component);
		}

		/**
		 * @brief Create a new component and add it to this actor
		 *
		 * @param factoryID The ID of the Component factory in with the CodeRegistry to create the component
		 *
		 * @throws ExistingValueException If a component of this type already exists on the actor
		 * @throws NonexistentValueException If the CodeRegistry does not have a Component actory registered for the provided ID
		 */
		Component& MountComponent(const std::string& factoryID, bool enabled = true);

		/**
		 * @brief Access a component on the actor
		 *
		 * @return The component
		 *
		 * @throws NonexistentValueException If a component of this type does not exist on the actor
		 */
		template<typename T>
			requires std::is_base_of_v<Component, T>
		T& GetComponent() const {
			Check<NonexistentValueException>(HasComponent<T>(), "A component of the type specified does not exist on the actor!");
			return dynamic_cast<T&>(*components.at(typeid(T)).component);
		}

		/**
		 * @brief Remove a component from the actor
		 *
		 * @throws NonexistentValueException If a component of this type does not exist on the actor
		 */
		template<typename T>
			requires std::is_base_of_v<Component, T>
		void UnmountComponent() {
			Check<NonexistentValueException>(HasComponent<T>(), "A component of the type specified does not exist on the actor!");
			components.erase(std::type_index(typeid(T)));
		}

		/**
		 * @brief Get a list of all the components on the actor
		 *
		 * @return References to all actor components
		 */
		std::unordered_map<std::type_index, Component*> GetAllComponents() const {
			return _ComponentGet([](const std::unique_ptr<Component>&) { return true; });
		}

		/**
		 * @brief Get a filtered list of all the components on the actor
		 *
		 * @param filter The filter function to check each component against, returning true for matching components
		 *
		 * @return References to all matching actor components
		 */
		template<typename F>
			requires std::is_invocable_r_v<bool, F, const std::unique_ptr<Component>&>
		std::unordered_map<std::type_index, Component*> GetComponentsFiltered(F filter) const {
			return _ComponentGet([&filter](const std::unique_ptr<Component>& c) { return filter(c); });
		}

		/**
		 * @brief Get all the children of the actor
		 *
		 * @return All child entities
		 */
		std::vector<ActorRef> GetAllChildren() const {
			return children;
		}

	  private:
		Actor(const std::string& name, ActorRef parent, ActorRef self, xg::Guid);
		friend class World;
		friend class Component;

		///@cond
		struct ComponentHandle {
			std::unique_ptr<Component> component;
			uint64_t generation = 0;

			ComponentHandle() = default;
			ComponentHandle(ComponentHandle&&) = default;
			ComponentHandle& operator=(ComponentHandle&&) = default;
			ComponentHandle(const ComponentHandle& o)
			  : component(std::make_unique<Component>(*o.component)), generation(0) {}
			ComponentHandle& operator=(const ComponentHandle& o) {
				if(this != &o) {
					component = std::make_unique<Component>(*o.component);
					generation = 0;
				}
				return *this;
			}
		};
		///@endcond

		ActorRef parent;
		ActorRef self;
		std::vector<ActorRef> children;
		Transform transform;
		mutable Transform worldTransformCached;
		mutable bool transformDirty = true;
		std::unordered_map<std::type_index, ComponentHandle> components;
		World* world;//NON-OWNING --- DO NOT FREE THIS!!!

		void _ComponentSetup(std::type_index type, std::unique_ptr<Component>&& ptr, bool enabled);
		std::unordered_map<std::type_index, Component*> _ComponentGet(std::function<bool(const std::unique_ptr<Component>&)> filter) const;

		bool active;
	};
}