#include "Utilities/AssetManager.hpp"

#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "Core/RuntimeHooks.hpp"
#include "3D/Model.hpp"
#include "Audio/AudioPlayer.hpp"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <array>

namespace Cacao {
	//Required static variable initialization
	AssetManager* AssetManager::instance = nullptr;
	bool AssetManager::instanceExists = false;

	//Singleton accessor
	AssetManager* AssetManager::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == nullptr) {
			//Create instance
			instance = new AssetManager();
			instanceExists = true;
		}

		return instance;
	}

	void _AH::_AM_Uncache_AHID(std::string id) {
		AssetManager::GetInstance()->UncacheAsset(id);
	}

	std::future<AssetHandle<Shader>> AssetManager::LoadShader(std::string assetID) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, assetID]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(assetID) && this->assetCache[assetID].lock()->GetType().compare("SHADER") == 0) return AssetHandle<Shader>(assetID, std::dynamic_pointer_cast<Shader>(this->assetCache[assetID].lock()));

			//Construct shader
			std::shared_ptr<Shader> asset = RTLoadShader(assetID);
			asset->CompileSync();

			//Add asset to cache
			this->assetCache.insert_or_assign(assetID, std::weak_ptr<Shader> {asset});

			//Construct and return asset handle
			return AssetHandle<Shader>(assetID, asset);
		});
	}

	std::future<AssetHandle<Mesh>> AssetManager::LoadMesh(std::string assetID) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, assetID]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(assetID) && this->assetCache[assetID].lock()->GetType().compare("MESH") == 0) return AssetHandle<Mesh>(assetID, std::dynamic_pointer_cast<Mesh>(this->assetCache[assetID].lock()));

			//Construct mesh
			std::shared_ptr<Mesh> asset = RTLoadMesh(assetID);
			asset->CompileSync();

			//Add asset to cache
			this->assetCache.insert_or_assign(assetID, std::weak_ptr<Mesh> {asset});

			//Construct and return asset handle
			return AssetHandle<Mesh>(assetID, asset);
		});
	}

	std::future<AssetHandle<Sound>> AssetManager::LoadSound(std::string assetID) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, assetID]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(assetID) && this->assetCache[assetID].lock()->GetType().compare("SOUND") == 0) return AssetHandle<Sound>(assetID, std::dynamic_pointer_cast<Sound>(this->assetCache[assetID].lock()));

			//Construct sound
			std::shared_ptr<Sound> asset = RTLoadSound(assetID);
			asset->CompileSync();

			//Add asset to cache
			this->assetCache.insert_or_assign(assetID, std::weak_ptr<Sound> {asset});

			//Construct and return asset handle
			return AssetHandle<Sound>(assetID, asset);
		});
	}

	std::future<AssetHandle<Font>> AssetManager::LoadFont(std::string assetID) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, assetID]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(assetID) && this->assetCache[assetID].lock()->GetType().compare("FONT") == 0) return AssetHandle<Font>(assetID, std::dynamic_pointer_cast<Font>(this->assetCache[assetID].lock()));

			//Construct font
			std::shared_ptr<Font> asset = RTLoadFont(assetID);
			asset->CompileSync();

			//Add asset to cache
			this->assetCache.insert_or_assign(assetID, std::weak_ptr<Font> {asset});

			//Construct and return asset handle
			return AssetHandle<Font>(assetID, asset);
		});
	}

	std::future<AssetHandle<Texture2D>> AssetManager::LoadTexture2D(std::string assetID) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, assetID]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(assetID) && this->assetCache[assetID].lock()->GetType().compare("TEX2D") == 0) return AssetHandle<Texture2D>(assetID, std::dynamic_pointer_cast<Texture2D>(this->assetCache[assetID].lock()));

			//Construct texture
			std::shared_ptr<Texture2D> asset = RTLoadTex2D(assetID);
			asset->CompileSync();

			//Add asset to cache
			this->assetCache.insert_or_assign(assetID, std::weak_ptr<Texture2D> {asset});

			//Construct and return asset handle
			return AssetHandle<Texture2D>(assetID, asset);
		});
	}

	std::future<AssetHandle<Cubemap>> AssetManager::LoadCubemap(std::string assetID) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, assetID]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(assetID) && this->assetCache[assetID].lock()->GetType().compare("CUBEMAP") == 0) return AssetHandle<Cubemap>(assetID, std::dynamic_pointer_cast<Cubemap>(this->assetCache[assetID].lock()));

			//Construct cubemap
			std::shared_ptr<Cubemap> asset = RTLoadCubemap(assetID);
			asset->CompileSync();

			//Add asset to cache
			this->assetCache.insert_or_assign(assetID, std::weak_ptr<Cubemap> {asset});

			//Construct and return asset handle
			return AssetHandle<Cubemap>(assetID, asset);
		});
	}

	std::future<AssetHandle<Skybox>> AssetManager::LoadSkybox(std::string assetID) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, assetID]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(assetID) && this->assetCache[assetID].lock()->GetType().compare("SKYBOX") == 0) return AssetHandle<Skybox>(assetID, std::dynamic_pointer_cast<Skybox>(this->assetCache[assetID].lock()));

			//Construct cubemap and skybox
			AssetHandle<Cubemap> cube("CUBEMAP4SKYBOX", RTLoadCubemap(assetID));
			cube->CompileSync();
			std::shared_ptr<Skybox> asset = std::make_shared<Skybox>(cube);

			//Add asset to cache
			this->assetCache.insert_or_assign(assetID, std::weak_ptr<Skybox> {asset});

			//Construct and return asset handle
			return AssetHandle<Skybox>(assetID, asset);
		});
	}
}
