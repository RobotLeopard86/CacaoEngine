#include "Utilities/AssetManager.hpp"

#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
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

	std::future<AssetHandle<Shader>> AssetManager::LoadShader(std::string definitionPath) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, definitionPath]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(definitionPath) && this->assetCache[definitionPath].lock()->GetType().compare("SHADER") == 0) return AssetHandle<Shader>(definitionPath, std::dynamic_pointer_cast<Shader>(this->assetCache[definitionPath].lock()));

			//Construct shader
			std::shared_ptr<Shader> asset = std::make_shared<Shader>(0, 0, ShaderSpec {});
			asset->CompileSync();

			//Add asset to cache
			this->assetCache.insert_or_assign(definitionPath, std::weak_ptr<Shader> {asset});

			//Construct and return asset handle
			return AssetHandle<Shader>(definitionPath, asset);
		});
	}

	std::future<AssetHandle<Texture2D>> AssetManager::LoadTexture2D(std::string path) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, path]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(path) && this->assetCache[path].lock()->GetType().compare("2DTEX") == 0) return AssetHandle<Texture2D>(path, std::dynamic_pointer_cast<Texture2D>(this->assetCache[path].lock()));

			//Construct an asset, add it to cache, and return a handle
			std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>(path);
			tex->CompileSync();
			this->assetCache.insert_or_assign(path, std::weak_ptr<Texture2D> {tex});
			return AssetHandle<Texture2D>(path, tex);
		});
	}

	std::future<AssetHandle<Cubemap>> AssetManager::LoadCubemap(std::string definitionPath) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, definitionPath]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(definitionPath) && this->assetCache[definitionPath].lock()->GetType().compare("CUBEMAP") == 0) return AssetHandle<Cubemap>(definitionPath, std::dynamic_pointer_cast<Cubemap>(this->assetCache[definitionPath].lock()));

			//Create and compile cubemap
			std::shared_ptr<Cubemap> asset = std::make_shared<Cubemap>(std::vector<std::string> {"a", "b", "c", "d", "e", "f"});
			asset->CompileSync();

			//Add asset to cache and return handle
			this->assetCache.insert_or_assign(definitionPath, std::weak_ptr<Cubemap> {asset});
			return AssetHandle<Cubemap>(definitionPath, asset);
		});
	}

	std::future<AssetHandle<Skybox>> AssetManager::LoadSkybox(std::string definitionPath) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, definitionPath]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(definitionPath) && this->assetCache[definitionPath].lock()->GetType().compare("SKYBOX") == 0) return AssetHandle<Skybox>(definitionPath, std::dynamic_pointer_cast<Skybox>(this->assetCache[definitionPath].lock()));

			//Create and compile skybox cubemap
			std::shared_ptr<Cubemap> cube = std::make_shared<Cubemap>(std::vector<std::string> {"a", "b", "c", "d", "e", "f"});
			cube->CompileSync();

			//Create skybox
			std::shared_ptr<Skybox> asset = std::make_shared<Skybox>(AssetHandle<Cubemap>(definitionPath, cube));

			//Add asset to cache and return handle
			this->assetCache.insert_or_assign(definitionPath, std::weak_ptr<Skybox> {asset});
			return AssetHandle<Skybox>(definitionPath, asset);
		});
	}

	std::future<AssetHandle<Mesh>> AssetManager::LoadMesh(std::string location) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, location]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(location) && this->assetCache[location].lock()->GetType().compare("MESH") == 0) return AssetHandle<Mesh>(location, std::dynamic_pointer_cast<Mesh>(this->assetCache[location].lock()));

			std::shared_ptr<Mesh> asset;
			asset->CompileSync();
			this->assetCache.insert_or_assign(location, std::weak_ptr<Mesh> {asset});

			//Return asset handle
			return AssetHandle<Mesh>(location, asset);
		});
	}

	std::future<AssetHandle<Sound>> AssetManager::LoadSound(std::string path) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, path]() {
			CheckException(std::filesystem::exists(path), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load sound from nonexistent file!");
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(path) && this->assetCache[path].lock()->GetType().compare("SOUND") == 0) return AssetHandle<Sound>(path, std::dynamic_pointer_cast<Sound>(this->assetCache[path].lock()));

			//Construct an asset, add it to cache, and return a handle
			std::shared_ptr<Sound> snd = std::make_shared<Sound>(path);
			snd->CompileSync();
			this->assetCache.insert_or_assign(path, std::weak_ptr<Sound> {snd});

			return AssetHandle<Sound>(path, snd);
		});
	}

	std::future<AssetHandle<Font>> AssetManager::LoadFont(std::string path) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, path]() {
			CheckException(std::filesystem::exists(path), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load font from nonexistent file!");
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(path) && this->assetCache[path].lock()->GetType().compare("FONT") == 0) return AssetHandle<Font>(path, std::dynamic_pointer_cast<Font>(this->assetCache[path].lock()));

			//Construct an asset, add it to cache, and return a handle
			std::shared_ptr<Font> font = std::make_shared<Font>(path);
			font->CompileSync();
			this->assetCache.insert_or_assign(path, std::weak_ptr<Font> {font});

			return AssetHandle<Font>(path, font);
		});
	}
}
