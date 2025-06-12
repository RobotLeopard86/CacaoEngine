#pragma once

#include "DllHelper.hpp"
#include "Resource.hpp"

#include "crossguid/guid.hpp"

#include <memory>

namespace Cacao {
	/**
	 * @brief Singleton for handling the loading of resources from a game bundle
	 */
	class CACAO_API ResourceManager {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static ResourceManager& Get();

		///@cond
		ResourceManager(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;
		///@endcond

		/**
		 * @brief Create and register a new resource (a ComponentExporter may not be created in this way)
		 *
		 * @param address The identifier component of the resource address to use
		 * @param pkg The identifier of the package to associate this resource with
		 * @param args The arguments to the resource constructor
		 *
		 * @throws NonexistentValueException If the requested package to associate with does not exist
		 * @throws ExistingValueException If the requested package already has an asset of that name
		 * @throws BadTypeException If a package already provides a resource with the provided address, but it is of a different type than is currently being instantiated
		 *
		 * @note This does not return the resource. The reason for this is because when a resource is registered it may be lower in the overlay stack.
		 */
		template<typename T, typename... Args>
			requires std::is_base_of_v<Resource, T> && (!std::is_same_v<BlobResource, T>) && std::is_constructible_v<T, Args&&...>
		void Instantiate(const std::string& address, const std::string& pkg, Args&&... args) = delete;

		/**
		 * @brief Load a resource by address
		 *
		 * @param address The resource address to load from
		 *
		 * @throws BadValueException If the template type does not match the loaded type of the asset
		 * @throws NonexistentValueException If there is no resource at the provided address
		 */
		template<typename T>
			requires std::is_base_of_v<Resource, T> && (!std::is_same_v<BlobResource, T>)
		ResourceHandle<T>& Load(const std::string& address) = delete;

		///@cond
		struct Impl;
		///@endcond
	  private:
		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		ResourceManager();
		~ResourceManager();
	};
}