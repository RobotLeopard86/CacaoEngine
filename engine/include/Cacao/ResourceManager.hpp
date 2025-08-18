#pragma once

#include "Asset.hpp"
#include "Exceptions.hpp"
#include "DllHelper.hpp"
#include "Resource.hpp"

#include <exception>
#include <memory>
#include <future>
#include <optional>
#include <typeindex>
#include <functional>
#include <typeinfo>

namespace Cacao {
	/**
	 * @brief A structure describing a request for a resource load from the ResourceManager
	 */
	struct ResourceQuery {
	  public:
		const std::string addr;										///<A well-formatted resource address (this does not mean the resource actually exists, just that the address is correctly formatted)
		const std::type_index type;									///<The requested resource type
		const std::function<void(std::shared_ptr<Resource>)> submit;///<The submission hook for successful load
		const std::function<void(std::exception_ptr ex)> fail;		///<The submission hook for a failed load

	  private:
		ResourceQuery(decltype(submit) submitFn, decltype(fail) failFn, std::type_index ti, const std::string& addr) : addr(addr), type(ti), submit(submitFn), fail(failFn) {}

		friend class ResourceManager;
	};

	///@brief A function that takes in a ResourceQuery and provides the appropriate resource
	using ResourceLoader = std::function<void(ResourceQuery&&)>;

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

			//Check cache
			std::shared_ptr<Resource> maybeCached = CheckCache(address);
			if(maybeCached) {
				//There is a cached resource
				try {
					//Try to cast the resource to the correct type and return it
					std::promise<std::shared_ptr<T>> p;
					std::shared_ptr<T> cached = std::dynamic_pointer_cast<T>(maybeCached);
					p.set_value(cached);
					return p.get_future().share();
				} catch(const std::bad_cast&) {
					Check<BadTypeException>(false, "Resource exists in cache but is not of the requested type!");
					return {};
				}
			}

			//Resource was not in cache, we need to load it
			//Check for a valid loader
			Check<BadStateException>(loader.has_value(), "No resource loader configured!");

			//Set up submission hooks
			std::promise<std::shared_ptr<T>> output;
			const auto submitFn = [&output](std::shared_ptr<Resource> r) {
				//Type-check the result
				if(std::shared_ptr<T> value = std::dynamic_pointer_cast<T>(std::move(r))) {
					//Set the result
					output.set_value(value);
				} else {
					//Make exception pointer to set for failure
					try {
						Check<BadTypeException>(false, "Resource loader returned improperly-typed result!");
					} catch(...) {
						output.set_exception(std::current_exception());
					}
				}
			};
			const auto failFn = [&output](std::exception_ptr ex) {
				output.set_exception(ex);
			};

			//Create resource query
			ResourceQuery query(submitFn, failFn, typeid(T), address);

			//Load resource
			loader.value()(std::move(query));

			//Return future
			return output.get_future().share();
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