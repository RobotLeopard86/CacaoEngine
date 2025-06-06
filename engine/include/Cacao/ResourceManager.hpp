#pragma once

#include "DllHelper.hpp"
#include "Resource.hpp"

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
		 *
		 * @throws NonexistentValueException If the requested package to associate with does not exist
		 * @throws BadValueException If the requested package already has an asset of that name
		 *
		 * @note This does not return the resource. The reason for this is because
		 */
		template<typename T>
			requires std::is_base_of_v<Resource, T> && (!std::is_same_v<BlobResource, T>)
		void Instantiate(const std::string& address, const std::string& pkg) = delete;

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
		ResourceTracker<T>& Load(const std::string& address) = delete;

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		ResourceManager();
		~ResourceManager();
	};
}