#pragma once

#include "Asset.hpp"
#include "Exceptions.hpp"
#include "DllHelper.hpp"
#include "Resource.hpp"
#include "Engine.hpp"

#include <memory>
#include <type_traits>
#include <string>
#include <typeindex>

namespace Cacao {
	//This is a helper type typedef because fully written out it's super long and nonsensical
	//To explain: This is the contained type of the returned unique_ptr type of a raw T (deref-ed for consistency)'s FetchData function
	///@cond
	template<typename T, typename R>
	using LoaderIntermediate = decltype(std::declval<std::remove_reference_t<T>>().template FetchData<R>(std::declval<std::string>()))::element_type;
	///@endcond

	/**
	 * @brief A concept that defines what a conforming resource loader looks like
	 *
	 * @tparam T The type of the loader object
	 * @tparam R A Resource type produced by the loader
	 *
	 * Concept Definition:
	 * @code {.cpp}
	 * template<typename T, typename R>
	 * concept Loader = std::is_base_of_v<Resource, R> && requires(T obj, const std::string& addr) {
	 * 		{ obj.template FetchData<R>(addr) } -> std::same_as<std::unique_ptr<LoaderIntermediate<T, R>>>;
	 * 		{ obj.template CreateResource<R>(std::unique_ptr<LoaderIntermediate<T, R>> {}) } -> std::same_as<std::shared_ptr<R>>;
	 * };
	 * @endcode
	 *
	 * These functions should expect to be invoked in the thread pool
	 */
	template<typename T, typename R>
	concept Loader = std::is_base_of_v<Resource, R> && requires(T obj, const std::string& addr) {
		{ obj.template FetchData<R>(addr) } -> std::same_as<std::unique_ptr<LoaderIntermediate<T, R>>>;
		{ obj.template CreateResource<R>(std::unique_ptr<LoaderIntermediate<T, R>> {}) } -> std::same_as<std::shared_ptr<R>>;
	};

	/**
	 * @brief A concept that describes a loader object that can handle a set of different resource types
	 *
	 * @tparam T The type of the loader object
	 * @tparam Rs... The types of Resources that can be handled
	 */
	template<typename T, typename... Rs>
	concept MultiLoader = (Loader<T, Rs> && ...);

	/**
	 * @brief Singleton for handling the loading of resources from a game bundle
	 * @warning This API is still very much under construction and will change as design stuff is worked out! Do not rely on the current version of this API!
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
		exathread::Future<std::shared_ptr<T>> Load(const std::string& address) {
			//Validate the address
			Check<BadValueException>(Resource::ValidateResourceAddr<T>(address), "Cannot load a resource from a malformed address string!");

			//Run load operation asynchronously
			return Engine::Get().GetThreadPool()->submit([this, address]() -> std::shared_ptr<T> {
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
				Check<BadStateException>(IsLoaderRegistered(typeid(T)), "No resource loader configured for the requested type!");

				//Try to load the asset
				std::shared_ptr<Resource> res = InvokeLoader(typeid(T), address);
				try {
					//Try to cast the resource to the correct type and return it
					return std::dynamic_pointer_cast<T>(res);
				} catch(const std::bad_cast&) {
					Check<BadTypeException>(false, "Resource was loade but the returned object is not of the requested type!");
					return {};
				}

				return {};
			});
		}

		/**
		 * @brief Set the resource loader for a given set of types
		 *
		 * @warning This function may only be called once per type. If the engine is not launched in standalone mode, this will be done automatically, and it will not be possible to change the loader for the default resource types.
		 *
		 * @param loader The resource loader to use
		 *
		 * @throws BadStateException If a loader has already been configured
		 */
		template<typename T, typename... Types>
			requires MultiLoader<std::remove_reference_t<T>, Types...>
		void ConfigureResourceLoader(T&& loader) {
			//For those unaware, this is a fold expression
			//What this does is it will run _ConfigureResourceLoader for each type in the Types pack
			((_ConfigureResourceLoader<T, Types>(std::move(loader))), ...);
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
		bool IsLoaderRegistered(std::type_index tp);
		std::shared_ptr<Resource> InvokeLoader(std::type_index tp, const std::string& addr);

		///@cond
		struct ErasedLoader {
			std::any loaderObj;
			std::function<std::shared_ptr<Resource>(const std::string&)> load;
		};

		template<typename T, typename R>
			requires Loader<std::remove_reference_t<T>, R>
		void _ConfigureResourceLoader(const T& loader) {
			Check<BadStateException>(!IsLoaderRegistered(typeid(R)), "A loader has already been configured for this type!");

			//Create erased loader object
			ErasedLoader el;
			el.loaderObj = loader;
			el.load = [loader](const std::string& addr) {
				std::unique_ptr<LoaderIntermediate<T, R>> intermediate = loader.template FetchData<R>(addr);
				return std::static_pointer_cast<Resource>(loader.template CreateResource<R>(std::move(intermediate)));
			};
		}
		///@endcond
	};
}