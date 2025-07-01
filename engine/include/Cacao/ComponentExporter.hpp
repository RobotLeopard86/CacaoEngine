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
	class CACAO_API ComponentExporter : public Resource {
	  public:
		const std::function<std::shared_ptr<Component>()> factory;
		const std::type_index type;

		/**
		 * @brief Create a new exporter
		 *
		 * @note This exists for both invocation sugar and stricter type-checking
		 */
		template<typename T>
			requires std::is_base_of_v<Component, T> && (!std::is_same_v<Script, T>)
		static ComponentExporter Create(std::function<std::shared_ptr<T>> factory, const std::string& addr, const std::string& pkg) {
			return ComponentExporter(addr, pkg, typeid(T), [factory]() { return std::static_pointer_cast<Component>(factory()); });
		}

	  private:
		ComponentExporter(const std::string& addr, const std::string& pkg, std::type_index tp, std::function<std::shared_ptr<Component>()> f)
		  : Resource(addr, pkg), factory(f), type(tp) {
			RegisterSelf();
		}

		//stub
		void RegisterSelf() {}
	};
}