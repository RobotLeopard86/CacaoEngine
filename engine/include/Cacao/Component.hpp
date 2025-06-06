#pragma once

#include "Actor.hpp"
#include "DllHelper.hpp"
#include "Resource.hpp"

#include <functional>
#include <memory>

namespace Cacao {
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
		const bool IsEnabled() {
			return actor.lock()->IsActive() && enabled;
		}

		/**
		 * @brief Activate or deactivate the component
		 *
		 * @param state The new activation state
		 */
		void SetEnabled(bool state) {
			enabled = state;
			OnEnableStateChange();
		}

		/**
		 * @brief Get the owning actor of this component
		 *
		 * @return A reference to the owning actor
		 *
		 * @throws NonexistentValueException If the actor no longer exists
		 */
		std::shared_ptr<Actor> GetOwner() {
			Check<NonexistentValueException>(!actor.expired(), "Cannot get expired Actor from Component!");
			return actor.lock();
		}

		/**
		 * @brief Runs when the component is first mounted on an Actor
		 *
		 * @note All setup should be performed here, <b>NOT</b> in the constructor. Only when this function is called is the component properly configured.
		 */
		virtual void OnMount() {};

		virtual ~Component() {}

	  protected:
		Component();

		virtual void OnEnableStateChange() {};

		std::weak_ptr<Actor> actor [[maybe_unused]];

		friend class Actor;

	  private:
		bool enabled;
	};

	///@cond
	class Script;
	///@endcond

	/**
	 * @brief A wrapper object for component creation used for exporting components from game code to make them visible to the engine
	 */
	class CACAO_API ComponentExporter : public Resource {
	  public:
		std::function<std::shared_ptr<Component>()> factory;
		std::type_index type;

		/**
		 * @brief Create a new exporter
		 *
		 * @note This exists for both invocation sugar and stricter type-checking
		 */
		template<typename T>
			requires std::is_base_of_v<Component, T> && (!std::is_same_v<Script, T>)
		static ComponentExporter Create(std::function<std::shared_ptr<T>> factory, const std::string& addr) {
			return ComponentExporter(addr, typeid(T), [factory]() { return std::static_pointer_cast<Component>(factory()); });
		}

	  private:
		ComponentExporter(const std::string& addr, std::type_index tp, std::function<std::shared_ptr<Component>()> f)
		  : Resource(addr), factory(f), type(tp) {}
	};
}