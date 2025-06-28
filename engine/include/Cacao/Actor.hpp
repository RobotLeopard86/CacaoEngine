#pragma once

#include "DllHelper.hpp"
#include "Transform.hpp"
#include "Exceptions.hpp"
#include "ComponentExporter.hpp"

#include <map>
#include <memory>
#include <optional>

#include "crossguid/guid.hpp"

namespace Cacao {
	class Component;

	class CACAO_API Actor : public std::enable_shared_from_this<Actor> {
	  public:
		/**
		 * @brief Create a new actor
		 *
		 * @note You can't use the constructor because the actor needs to have an accesible shared_ptr to itself
		 *
		 * @param name The initial name of the actor
		 * @param parent The initial actor parent (set this to nullopt to create an orphaned actor)
		 *
		 * @return A shared_ptr to the actor
		 */
		static std::shared_ptr<Actor> Create(const std::string& name, std::optional<std::shared_ptr<Actor>> parent);

		std::string name;	///<The human-readable name of the actor
		xg::Guid guid;		///<Actor ID, unique
		Transform transform;///<Actor transform relative to parent

		/**
		 * @brief Calculate the world-space transformation matrix of the actor
		 *
		 * @return The world-space transformation matrix
		 */
		glm::mat4 GetWorldTransformMatrix();

		/**
		 * @brief Access the parent of this actor
		 *
		 * @return The parent actor, or an empty pointer if this is an orphaned actor
		 */
		std::shared_ptr<Actor> GetParent() {
			return parentPtr;
		}

		/**
		 * @brief Check if the actor is active
		 * @details This takes into account both if the actor is enabled and its parent Actor is active
		 *
		 * @note This will return false if the parent Actor is inactive
		 */
		bool IsActive() {
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
		 * @param newParent The new parent of this actor, or nullopt to orphan the actor
		 */
		void Reparent(std::optional<std::shared_ptr<Actor>> newParent);

		/**
		 * @brief Create a new component and add it to this actor
		 *
		 * @param args The arguments to the component constructor
		 *
		 * @throws ContainerException If a component of this type already exists on the actor
		 */
		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T> && std::is_constructible_v<T, Args&&...>
		void MountComponent(Args&&... args) {
			Check<ContainerException>(!components.contains(std::type_index(typeid(T))), "A component of the type specified already exists on the actor!");
			components.insert_or_assign(std::type_index(typeid(T)), std::make_shared<T>(std::forward<Args...>(args...)));
			PostMountComponent(components[std::type_index(typeid(T))]);
		}

		/**
		 * @brief Create a new component and add it to this actor
		 *
		 * @param exporter The handle to the ComponentExporter from which to construct the component
		 *
		 * @throws ContainerException If a component of this type already exists on the actor
		 */
		void MountComponent(std::shared_ptr<ComponentExporter> exporter) {
			Check<ContainerException>(!components.contains(std::type_index(exporter->type)), "A component of the type specified already exists on the actor!");
			components.insert_or_assign(std::type_index(std::type_index(exporter->type)), exporter->factory());
			PostMountComponent(components[std::type_index(std::type_index(exporter->type))]);
		}

		/**
		 * @brief Check if a component is on an actor
		 *
		 * @return Whether a component of the type is on the actor
		 */
		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T>
		bool HasComponent() {
			return components.contains(std::type_index(typeid(T)));
		}

		/**
		 * @brief Access a component on the actor
		 *
		 * @return The component
		 *
		 * @throws ContainerException If a component of this type does not exist on the actor
		 */
		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T>
		std::shared_ptr<T> GetComponent() {
			Check<ContainerException>(components.contains(std::type_index(typeid(T))), "A component of the type specified does not exist on the actor!");
			return std::dynamic_pointer_cast<T>(components.at(std::type_index(typeid(T))));
		}

		/**
		 * @brief Delete a component from the actor
		 *
		 * @throws ContainerException If a component of this type does not exist on the actor
		 */
		template<typename T, typename... Args>
			requires std::is_base_of_v<Component, T>
		void DeleteComponent() {
			Check<ContainerException>(components.contains(std::type_index(typeid(T))), "A component of the type specified does not exist on the actor!");
			components.erase(std::type_index(typeid(T)));
		}

		/**
		 * @brief Get a copy of all the components on the actor
		 *
		 * @note This doesn't actually copy the components, just their pointers, but this does increment the reference count
		 *
		 * @return All actor components
		 */
		std::map<std::type_index, std::shared_ptr<Component>> GetAllComponents() {
			return components;
		}

		/**
		 * @brief Get all the children of the actor
		 *
		 * @return All child entities
		 */
		std::vector<std::shared_ptr<Actor>> GetAllChildren() {
			return children;
		}

		~Actor();

	  private:
		Actor(const std::string& name, std::optional<std::shared_ptr<Actor>> parent);

		std::shared_ptr<Actor> parentPtr;
		std::map<std::type_index, std::shared_ptr<Component>> components;
		std::vector<std::shared_ptr<Actor>> children;

		void PostMountComponent(std::shared_ptr<Component> c);
		void NotifyFunctionallyActiveStateChanged();

		bool active, functionallyActive;
	};
}