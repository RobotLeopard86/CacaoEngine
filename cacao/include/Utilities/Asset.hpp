#pragma once

#include "Core/Log.hpp"

#include <memory>
#include <string>
#include <future>

namespace Cacao {
	/**
	 * @brief Base asset type
	 */
	class Asset {
	  public:
		/**
		 * @brief Compile the raw asset data into a format that can be used asynchronously
		 *
		 * @return A future that will resolve when compilation is done
		 *
		 * @throws Exception If the asset was already compiled
		 */
		virtual std::shared_future<void> CompileAsync() {
			Logging::EngineLog("Cannot compile base asset type!", LogLevel::Error);
			return {};
		}

		/**
		 * @brief Compile the raw asset data into a format that can be used synchronously
		 *
		 * @throws Exception If the asset was already compiled
		 */
		virtual void CompileSync() {
			Logging::EngineLog("Cannot compile base asset type!", LogLevel::Error);
		}

		/**
		 * @brief Delete the compiled asset data
		 *
		 * @throws Exception If the asset was not compiled
		 */
		virtual void Release() {
			Logging::EngineLog("Cannot release base asset type!", LogLevel::Error);
		}

		/**
		 * @brief Check if the asset is compiled
		 *
		 * @return Whether the asset is compiled or not
		 */
		virtual bool IsCompiled() const {
			return compiled;
		}

		///@brief Get the asset type (not useful here because this is the base asset)
		virtual std::string GetType() const {
			return "N/A";
		}

		//Virtual destructor
		virtual ~Asset() {}

	  protected:
		bool compiled;

		//Constructor for initialization purposes
		Asset(bool initiallyCompiled)
		  : compiled(initiallyCompiled) {}
	};

	//Actually runs the uncaching, can't be implemented here due to cyclical references
	namespace _AH {
		void _AM_Uncache_AHID(std::string id);
	}

	/**
	 * @brief Reference-counted handle to an asset
	 */
	template<typename T>
	class AssetHandle {
	  public:
		/**
		 * @brief Construct an asset handle with an ID and asset
		 * @note Prefer to use AssetManager::Load* methods to get AssetHandles.
		 *
		 * @param id The asset ID
		 * @param asset The asset, which must have a type that is a subclass of Asset
		 */
		AssetHandle(const std::string& id, std::shared_ptr<T> asset)
		  : asset(asset), id(id), isNullHandle(false) {
			static_assert(std::is_base_of<Asset, T>(), "Cannot construct asset handle with non-subclass of Asset!");
		}

		/**
		 * @brief Construct a "null" AssetHandle
		 *
		 * @note For use by the engine only
		 */
		AssetHandle()
		  : asset(nullptr), id("nullptr_HANDLE_DONT_USE_ME"), isNullHandle(true) {}

		/**
		 * @brief Destroy the asset handle
		 * @details If this was the last reference to the asset, uncache it
		 */
		~AssetHandle() {
			if(asset.use_count() == 1) {
				_AH::_AM_Uncache_AHID(id);
			}
		}

		/**
		 * @brief Access the underlying shared_ptr
		 * @details This is done so the AssetHandle can act in place of the shared_ptr
		 */
		operator std::shared_ptr<T>() {
			return asset;
		}

		/**
		 * @brief Access the managed asset
		 *
		 * @return The managed asset shared_ptr
		 */
		std::shared_ptr<T>& operator->() {
			return asset;
		}

		/**
		 * @brief Access the managed asset in a const manner
		 *
		 * @return A const shared_ptr for the managed asset
		 */
		const std::shared_ptr<T>& operator->() const {
			return asset;
		}

		/**
		 * @brief Get the managed asset
		 * @details This can be used when you need the shared_ptr value and can't use the operator shared_ptr<T> or the arrow operator
		 */
		std::shared_ptr<T>& GetManagedAsset() {
			return asset;
		}

		/**
		 * @brief Get the asset ID
		 *
		 * @return The asset ID
		 */
		std::string GetID() {
			return id;
		}

		/**
		 * @brief Check if this is a "null" asset handle
		 *
		 * @return Whether this is a "null" handle or not
		 */
		bool IsNull() {
			return isNullHandle;
		}

	  private:
		std::shared_ptr<T> asset;
		std::string id;
		bool isNullHandle;
	};
}