#pragma once

#include "Cacao/ComponentExporter.hpp"
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
		 * @brief Load a resource by address
		 *
		 * @param address The resource address to load from
		 *
		 * @throws BadValueException If the template type does not match the loaded type of the resource
		 * @throws NonexistentValueException If there is no resource at the provided address
		 *
		 * @return A handle to the resource
		 */
		template<typename T>
			requires std::is_base_of_v<Resource, T> && (!std::is_same_v<BlobResource, T>)
		std::shared_ptr<T> Load(const std::string& address) = delete;

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