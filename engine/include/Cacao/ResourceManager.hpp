#pragma once

#include "Asset.hpp"
#include "ThreadPool.hpp"
#include "Exceptions.hpp"
#include "DllHelper.hpp"
#include "Resource.hpp"

#include <memory>
#include <future>
#include <optional>
#include <typeindex>
#include <functional>
#include <typeinfo>
#include <string>
#include <functional>

namespace Cacao {
	/**
	 * @brief A function that takes in a ResourceQuery and provides the appropriate resource
	 *
	 * @note This function should expect to be invoked in a synchronous manner, though on a worker thread in the standard thread pool.
	 *
	 * @warning If your loader requires @b any form of IO-bound task (e.g. filesystem or networking), @b please perform that task
	 *
	 * @param addr The resource address to load
	 * @param type The type of object expected as a result
	 */
	using ResourceLoader = std::function<void(const std::string& addr, const std::type_index type)>;

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
		 * @throws BadValueException If the the address string is malformed
		 * @throws BadTypeException If the template type does not match the loaded type of the resource
		 * @throws BadStateException If no ResourceLoader has been configured
		 * @throws NonexistentValueException If there is no resource at the provided address
		 *
		 * @return A future that will return a handle to the resource when completed
		 */
		template<typename T>
			requires std::is_base_of_v<Resource, T> && (!std::is_same_v<BlobResource, T>) && (!std::is_same_v<Asset, T>)
		std::shared_future<std::shared_ptr<T>> Load(const std::string& address) {
			//Validate the address
			Check<BadValueException>(Resource::ValidateResourceAddr<T>(address), "Cannot load a resource from a malformed address string!");

			//Run load operation asynchronously
			return ThreadPool::Get().Exec([this, &address]() -> std::shared_ptr<T> {
				//Check cache
				std::shared_ptr<Resource> maybeCached = CheckCache(address);
				if(maybeCached) {
					//There is a cached resource
					try {
						//Try to cast the resource to the correct type and return it
						return std::dynamic_pointer_cast<T>(maybeCached);
					} catch(const std::bad_cast&) {
						Check<BadTypeException>(false, "Resource exists in cache but is not of the requested type!");
						return {};
					}
				}

				//Resource was not in cache, we need to load it
				//Check for a valid loader
				Check<BadStateException>(loader.has_value(), "No resource loader configured!");

				//Try to load the asset
				loader.value()(address, std::type_index(typeid(T)));
			});
		}

		/**
		 * @brief Set the resource loader
		 *
		 * @warning This function may only be called once. If the engine is not launched in standalone mode, this will be done automatically, and it will not be possible to change the loader.
		 *
		 * @param loader The resource loader to use
		 *
		 * @throws BadStateException If a loader has already been configured
		 */
		void ConfigureResourceLoader(ResourceLoader loader) {
			Check<BadStateException>(!this->loader.has_value(), "A loader has already been configured!");
			this->loader = loader;
		}

		///@cond
		struct Impl;
		///@endcond
	  private:
		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		ResourceManager();
		~ResourceManager();

		std::shared_ptr<Resource> CheckCache(const std::string& addr);

		std::optional<ResourceLoader> loader;
	};
}