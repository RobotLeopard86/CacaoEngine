#pragma once

#include "DllHelper.hpp"
#include "Resource.hpp"

#include <functional>
#include <memory>
#include <typeindex>

namespace Cacao {
	///@cond
	class Script;
	class Component;
	///@endcond

	/**
	 * @brief A wrapper object for component creation used for exporting components from game code to make them visible to the engine
	 */
	class CACAO_API ComponentExporter : public Resource, std::enable_shared_from_this<ComponentExporter> {
	  public:
		const std::type_index type;

		/**
		 * @brief Create a new exporter
		 *
		 * @param factory A function that constructs the component on demand
		 * @param addr The resource address identifier to associate with the component exporter
		 */
		template<typename T>
			requires std::is_base_of_v<Component, T> && (!std::is_same_v<Script, T>)
		static ComponentExporter Create(std::function<std::shared_ptr<T>> factory, const std::string& addr) {
			return ComponentExporter(addr, typeid(T), [factory]() { return std::static_pointer_cast<Component>(factory()); });
		}

		/**
		 * @brief Create a new instance of the component
		 *
		 * @return Component instance pointer
		 */
		std::shared_ptr<Component> Instantiate();

	  private:
		ComponentExporter(const std::string& addr, std::type_index tp, std::function<std::shared_ptr<Component>()> f)
		  : Resource(addr), type(tp), factory(f) {}

		const std::function<std::shared_ptr<Component>()> factory;
	};
}