#pragma once

#include "DllHelper.hpp"
#include "Component.hpp"

#include <functional>
#include <memory>
#include <type_traits>
#include <typeindex>

namespace Cacao {
	/**
	 * @brief Registry for client object factories
	 */
	class CACAO_API CodeRegistry {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static CodeRegistry& Get();

		///@cond
		CodeRegistry(const CodeRegistry&) = delete;
		CodeRegistry(CodeRegistry&&) = delete;
		CodeRegistry& operator=(const CodeRegistry&) = delete;
		CodeRegistry& operator=(CodeRegistry&&) = delete;
		///@endcond

		///@cond
		struct Impl;
		///@endcond

		/**
		 * @brief Register a factory for a type of code object
		 *
		 * @tparam Code object base type
		 *
		 * @param id The ID of the object factory
		 * @param factory A factory function to make the object
		 * @param type The type of the object that will be returned from the factory
		 *
		 * @throws ExistingValueException If another factory of the same base type already has the provided ID
		 */
		template<typename T>
			requires std::is_same_v<Component, T>
		void RegisterFactory(const std::string& id, std::function<T*()> factory, std::type_index type);

		/**
		 * @brief Check if a factory for a given type of code object has been registered
		 *
		 * @param id
		 * @param factory
		 * @param type
		 * @return requires
		 */
		template<typename T>
			requires std::is_same_v<Component, T>
		bool HasFactory(const std::string& id);

		/**
		 * @brief Create an object instance using a registered factory
		 *
		 * @tparam Code object base type
		 *
		 * @param id The ID of the factory to use
		 *
		 * @returns A pair containing the instantiated object and its actual type
		 *
		 * @throws NonexistentValueException If no factory has been registered with the provided ID for the provided base type
		 */
		template<typename T>
			requires std::is_same_v<Component, T>
		std::pair<std::shared_ptr<T>, std::type_index> Instantiate(const std::string& id);

	  private:
		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		CodeRegistry();
		~CodeRegistry();
	};

	//Declare explicitly allowed specializations
	///@cond
	template<>
	void CodeRegistry::RegisterFactory<Component>(const std::string& id, std::function<Component*()> factory, std::type_index type);

	template<>
	std::pair<std::shared_ptr<Component>, std::type_index> CodeRegistry::Instantiate<Component>(const std::string& id);
	///@endcond
}