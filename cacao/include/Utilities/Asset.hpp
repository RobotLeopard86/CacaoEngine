#pragma once

#include "Core/Log.hpp"

#include <memory>
#include <string>
#include <future>

namespace Cacao {
	//Base asset type
	class Asset {
	  public:
		//Compile asset to be used later
		virtual std::shared_future<void> Compile() {
			Logging::EngineLog("Cannot compile base asset type!");
			return {};
		}
		//Delete asset when no longer needed
		virtual void Release() {
			Logging::EngineLog("Cannot release base asset type!");
		}
		//Check if compiled
		virtual bool IsCompiled() {
			return compiled;
		}

		//Get asset type
		virtual std::string GetType() {
			return "N/A";
		}

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

	//Handle to an asset
	template<typename T>
	class AssetHandle {
	  public:
		//Construct an asset handle with an ID and asset
		AssetHandle(const std::string& id, std::shared_ptr<T> asset)
		  : id(id), asset(asset) {
			static_assert(std::is_base_of<Asset, T>(), "Cannot construct asset handle with non-subclass of Asset!");
		}

		//Construct a "null" handle (used for failed asset load calls)
		AssetHandle()
		  : id("NULL_HANDLE_DONT_USE"), asset(nullptr) {}

		//Remove from asset cache if this is the last handle
		~AssetHandle() {
			if(asset.use_count() == 1) {
				_AH::_AM_Uncache_AHID(id);
			}
		}

		//Allows this handle to be used directly in place of the asset type
		operator std::shared_ptr<T>() {
			return asset;
		}

		//Do stuff with the asset
		std::shared_ptr<T>& operator->() {
			return asset;
		}

		//Get the asset manually
		std::shared_ptr<T>& GetManagedAsset() {
			return asset;
		}

		//Get the ID of the asset
		std::string GetID() {
			return id;
		}

	  private:
		std::shared_ptr<T> asset;
		std::string id;
	};
}