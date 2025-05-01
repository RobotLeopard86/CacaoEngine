#pragma once

#include "3D/Mesh.hpp"
#include "3D/Skybox.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Audio/Sound.hpp"
#include "UI/Font.hpp"
#include "Core/Exception.hpp"
#include "Core/DllHelper.hpp"
#include "Asset.hpp"
#include "AssetLoader.hpp"

#include <future>

namespace Cacao {
	/**
	 * @brief Manages the loading of assets and the asset cache
	 */
	class CACAO_API AssetManager {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static AssetManager* Get();

		/**
		 * @brief Set the asset loader to use
		 *
		 * @note For use by the engine only
		 *
		 * @param loader The asset loader
		 */
		void SetLoader(std::shared_ptr<AssetLoader> loader);

		/**
		 * @brief Load a shader by its asset ID
		 *
		 * @param assetID The ID of the shader to load
		 *
		 * @throws Exception If the asset loader has not been configured, the asset ID is invalid or the shader fails to compile
		 * @see Shader::Compile
		 */
		std::future<AssetHandle<Shader>> LoadShader(std::string assetID);

		/**
		 * @brief Load a 2D texture by its asset ID
		 *
		 * @param assetID The ID of the texture to load
		 *
		 * @throws Exception If the asset loader has not been configured, the asset ID is invalid or the texture fails to compile
		 * @see Texture2D::Compile
		 */
		std::future<AssetHandle<Texture2D>> LoadTexture2D(std::string assetID);

		/**
		 * @brief Load a cubemap by its asset ID
		 *
		 * @param assetID The ID of the cubemap to load
		 *
		 * @throws Exception If the asset loader has not been configured, the asset ID is invalid or the cubemap fails to compile
		 * @see Cubemap::Compile
		 */
		std::future<AssetHandle<Cubemap>> LoadCubemap(std::string assetID);

		/**
		 * @brief Load a skybox from a cubemap by its asset ID
		 *
		 * @param assetID The ID of the cubemap to load and use
		 *
		 * @throws Exception If the asset loader has not been configured, the asset ID is invalid or the cubemap fails to compile
		 * @see Cubemap::Compile,LoadCubemap
		 */
		std::future<AssetHandle<Skybox>> LoadSkybox(std::string assetID);

		/**
		 * @brief Load a mesh by its asset ID
		 *
		 * @param assetID The ID of the mesh to load
		 *
		 * @throws Exception If the asset loader has not been configured, the asset ID is invalid or the mesh fails to compile
		 * @see Model::Model,Mesh::Compile
		 */
		std::future<AssetHandle<Mesh>> LoadMesh(std::string assetID);

		/**
		 * @brief Load a sound by its asset ID
		 *
		 * @param assetID The ID of the sound to load
		 *
		 * @throws Exception If the asset loader has not been configured, the asset ID is invalid or the sound fails to compile
		 * @see Sound::Compile
		 */
		std::future<AssetHandle<Sound>> LoadSound(std::string assetID);

		/**
		 * @brief Load a font face by its asset ID
		 *
		 * @param assetID The ID of the shader to load
		 *
		 * @throws Exception If the asset loader has not been configured, the asset ID is invalid or the font fails to compile
		 */
		std::future<AssetHandle<Font>> LoadFont(std::string assetID);

		/**
		 * @brief Remove an asset from the cache
		 *
		 * @note For use by the engine and runtimes only
		 *
		 * @param assetID The ID of the asset to remove from the cache
		 *
		 * @note If the specified asset ID does not exist in the cache, this call is ignored
		 */
		void UncacheAsset(std::string assetID) {
			if(!assetCache.contains(assetID)) {
				Logging::EngineLog("Uncaching of asset not in asset cache requested, ignoring...", LogLevel::Warn);
				return;
			}
			assetCache.erase(assetID);
		}

		/**
		 * @brief Put an asset into the cache
		 *
		 * @note For use by the engine and runtimes only
		 *
		 * @param id Asset ID to put in the cache
		 * @param handle Asset pointer to associate with the ID
		 *
		 * @note If the specified asset ID does not exist in the cache, this call is ignored
		 */

		void CacheAsset(std::string assetID, std::weak_ptr<Asset> handle) {
			if(assetCache.contains(assetID)) {
				Logging::EngineLog("Caching of asset already in asset cache requested, ignoring...", LogLevel::Warn);
				return;
			}
			assetCache.insert_or_assign(assetID, handle);
		}

		/**
		 * @brief Get an AssetHandle from the asset's stored pointer
		 * @details This is used for assets to be able to retrieve their own handle, typically for passing to another object
		 *
		 * @note For use by the engine only
		 *
		 * @param ptr A raw pointer to the asset (typically obtained from this)
		 *
		 * @return An asset handle, which will be a null handle if the asset was not found in the cache
		 */
		template<typename T>
		AssetHandle<T> GetHandleFromPointer(T* ptr) {
			if(auto it = std::find_if(assetCache.begin(), assetCache.end(), [ptr](auto& kv) {
				   return kv.second.lock().get() == ptr;
			   });
				it != assetCache.end()) {
				return AssetHandle<T>(it->first, std::dynamic_pointer_cast<Shader>(it->second.lock()));
			} else {
				return AssetHandle<T>();
			}
		}

	  private:
		//Singleton members
		static AssetManager* instance;
		static bool instanceExists;

		//Asset cache
		std::map<std::string, std::weak_ptr<Asset>> assetCache;

		//Asset loader
		std::shared_ptr<AssetLoader> loader;

		AssetManager() {}
	};
}
