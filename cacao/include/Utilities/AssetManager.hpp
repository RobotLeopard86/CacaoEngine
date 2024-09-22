#pragma once

#include "3D/Mesh.hpp"
#include "3D/Skybox.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Audio/Sound.hpp"
#include "UI/Font.hpp"
#include "Core/Exception.hpp"
#include "Asset.hpp"

#include <future>

namespace Cacao {
	//Game asset manager
	class AssetManager {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static AssetManager* GetInstance();

		//Load a shader with a path to a shader definition file
		std::future<AssetHandle<Shader>> LoadShader(std::string definitionPath);

		//Load a 2D texture from a file
		std::future<AssetHandle<Texture2D>> LoadTexture2D(std::string path);
		//Load a cubemap from a cubemap definition file
		std::future<AssetHandle<Cubemap>> LoadCubemap(std::string definitionPath);

		//Load a skybox with a cubemap created from a cubemap definition file
		std::future<AssetHandle<Skybox>> LoadSkybox(std::string definitionPath);

		//Load a mesh from a model file
		//Location format: <model path>:<mesh ID>
		std::future<AssetHandle<Mesh>> LoadMesh(std::string location);

		//Load a sound from a file
		std::future<AssetHandle<Sound>> LoadSound(std::string path);

		//Load a font face from a file
		std::future<AssetHandle<Font>> LoadFont(std::string path);

		//Remove an asset from the cache by ID
		//Generally should not be used (exists for assets to deregister themselves)
		void UncacheAsset(std::string assetID) {
			CheckException(assetCache.contains(assetID), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Unable to remove uncached asset from cache!")
			assetCache.erase(assetID);
		}

	  private:
		//Singleton members
		static AssetManager* instance;
		static bool instanceExists;

		//Asset cache
		std::map<std::string, std::weak_ptr<Asset>> assetCache;

		AssetManager() {}
	};
}
