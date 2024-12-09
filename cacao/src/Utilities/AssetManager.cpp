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
			CheckException(std::filesystem::exists(definitionPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load shader from nonexistent definition file!");
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(definitionPath) && this->assetCache[definitionPath].lock()->GetType().compare("SHADER") == 0) return AssetHandle<Shader>(definitionPath, std::dynamic_pointer_cast<Shader>(this->assetCache[definitionPath].lock()));

			//Load and validate definition file
			YAML::Node dfNode = YAML::LoadFile(definitionPath);
			CheckException(dfNode.IsMap(), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing shader definition: File is not a map!");
			CheckException(dfNode["vertex"].IsScalar(), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing shader definition: File does not contain required 'vertex' attribute or it is not a scalar!");
			CheckException(dfNode["fragment"].IsScalar(), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing shader definition: File does not contain required 'fragment' attribute or it is not a scalar!");
			CheckException(std::filesystem::exists(dfNode["vertex"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing shader definition: 'vertex' attribute references nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["fragment"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing shader definition: 'fragment' attribute references nonexistent file!");
			CheckException(!dfNode["spec"] || (dfNode["spec"] && dfNode["spec"].IsSequence()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing shader definition: 'spec' attribute is not a sequence!");

			//Validate and try to build spec
			int specEntryCounter = 1;
			ShaderSpec spec;
			constexpr std::array<const char*, 5> validTypes {"int", "uint", "float", "texture"};

			for(auto node : dfNode["spec"]) {
				//Convience string generator function
				auto genErr = [specEntryCounter](std::string err) {
					std::stringstream stream;
					stream << "While parsing shader definition spec, entry #" << specEntryCounter << ": " << err;
					return stream.str();
				};

				//Validate
				CheckException(node.IsMap(), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), genErr("Spec entry is not a map!"));
				CheckException(node["name"].IsScalar(), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), genErr("'name' attribute is not a scalar!"));
				CheckException(node["sizex"], Exception::GetExceptionCodeFromMeaning("InvalidYAML"), genErr("'sizex' attribute does not exist!"));
				CheckException(node["sizex"].IsScalar() && node["sizex"].as<int>(INT_MIN) != INT_MIN, Exception::GetExceptionCodeFromMeaning("InvalidYAML"), genErr("'sizex' attribute is not a number!"));
				CheckException((node["sizex"].as<int>(INT_MIN) > 0 && node["sizex"].as<int>(INT_MIN) < 5), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), genErr("'sizex' attribute is not a value from 1-4!"));
				CheckException(node["sizey"], Exception::GetExceptionCodeFromMeaning("InvalidYAML"), genErr("'sizey' attribute does not exist!"));
				CheckException(node["sizey"].IsScalar() && node["sizey"].as<int>(INT_MIN) != INT_MIN, Exception::GetExceptionCodeFromMeaning("InvalidYAML"), genErr("'sizey' attribute is not a number!"));
				CheckException((node["sizey"].as<int>(INT_MIN) > 0 && node["sizey"].as<int>(INT_MIN) < 5), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), genErr("'sizey' attribute is not a value from 1-4!"));
				CheckException(node["type"], Exception::GetExceptionCodeFromMeaning("InvalidYAML"), genErr("'type' attribute does not exist!"));
				CheckException(node["type"].IsScalar() && std::find(validTypes.cbegin(), validTypes.cend(), node["type"].Scalar()) != validTypes.cend(), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), genErr("'type' attribute is not a valid type!"));

				//Build spec entry
				ShaderItemInfo inf;
				inf.entryName = node["name"].Scalar();
				inf.size = {node["sizex"].as<int>(INT_MIN), node["sizey"].as<int>(INT_MIN)};

				//Find index of value in valid type array
				int idx = 0;
				for(; idx < validTypes.size(); idx++) {
					if(node["type"].Scalar().compare(validTypes[idx]) == 0) break;
				}

				switch(idx) {
					case 0://int
						inf.type = SpvType::Int;
						break;
					case 1://uint
						inf.type = SpvType::UInt;
						break;
					case 2://float
						inf.type = SpvType::Float;
						break;
					case 3://image
						inf.type = SpvType::SampledImage;
						break;
					default:
						inf.type = SpvType::Unknown;
						break;
				}

				//Add entry to spec
				spec.push_back(inf);
			}

			//Construct shader
			std::shared_ptr<Shader> asset = std::make_shared<Shader>(dfNode["vertex"].Scalar(), dfNode["fragment"].Scalar(), spec);
			asset->CompileSync();

			//Add asset to cache
			this->assetCache.insert_or_assign(definitionPath, std::weak_ptr<Shader> {asset});

			//Construct and return asset handle
			return AssetHandle<Shader>(definitionPath, asset);
		});
	}

	std::future<AssetHandle<Texture2D>> AssetManager::LoadTexture2D(std::string path) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, path]() {
			CheckException(std::filesystem::exists(path), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load 2D texture from nonexistent file!");
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
			CheckException(std::filesystem::exists(definitionPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load cubemap from nonexistent definition file!");
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(definitionPath) && this->assetCache[definitionPath].lock()->GetType().compare("CUBEMAP") == 0) return AssetHandle<Cubemap>(definitionPath, std::dynamic_pointer_cast<Cubemap>(this->assetCache[definitionPath].lock()));

			//Load and validate definition file
			YAML::Node dfNode = YAML::LoadFile(definitionPath);
			CheckException(dfNode.IsMap(), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: File is not a map!");
			CheckException((dfNode["x+"] && dfNode["x+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'x+' field is nonexistent or not a scalar!");
			CheckException((dfNode["x-"] && dfNode["x+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'x-' field is nonexistent or not a scalar!");
			CheckException((dfNode["y+"] && dfNode["y+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'y+' field is nonexistent or not a scalar!");
			CheckException((dfNode["y-"] && dfNode["y+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'y-' field is nonexistent or not a scalar!");
			CheckException((dfNode["z+"] && dfNode["z+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'z+' field is nonexistent or not a scalar!");
			CheckException((dfNode["z-"] && dfNode["z+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'z-' field is nonexistent or not a scalar!");
			CheckException(std::filesystem::exists(dfNode["x+"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'x+' field refers to a nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["x-"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'x-' field refers to a nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["y+"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'y+' field refers to a nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["y-"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'y-' field refers to a nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["z+"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'z+' field refers to a nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["z-"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing cubemap definition: 'z-' field refers to a nonexistent file!");

			//Create and compile cubemap
			std::shared_ptr<Cubemap> asset = std::make_shared<Cubemap>(std::vector<std::string> {dfNode["x+"].Scalar(), dfNode["x-"].Scalar(), dfNode["y+"].Scalar(), dfNode["y-"].Scalar(), dfNode["z+"].Scalar(), dfNode["z-"].Scalar()});
			asset->CompileSync();

			//Add asset to cache and return handle
			this->assetCache.insert_or_assign(definitionPath, std::weak_ptr<Cubemap> {asset});
			return AssetHandle<Cubemap>(definitionPath, asset);
		});
	}

	std::future<AssetHandle<Skybox>> AssetManager::LoadSkybox(std::string definitionPath) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, definitionPath]() {
			CheckException(std::filesystem::exists(definitionPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load skybox from nonexistent definition file!");
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(definitionPath) && this->assetCache[definitionPath].lock()->GetType().compare("SKYBOX") == 0) return AssetHandle<Skybox>(definitionPath, std::dynamic_pointer_cast<Skybox>(this->assetCache[definitionPath].lock()));

			//Load and validate definition file
			YAML::Node dfNode = YAML::LoadFile(definitionPath);
			CheckException(dfNode.IsMap(), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: File is not a map!");
			CheckException((dfNode["x+"] && dfNode["x+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'x+' field is nonexistent or not a scalar!");
			CheckException((dfNode["x-"] && dfNode["x+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'x-' field is nonexistent or not a scalar!");
			CheckException((dfNode["y+"] && dfNode["y+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'y+' field is nonexistent or not a scalar!");
			CheckException((dfNode["y-"] && dfNode["y+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'y-' field is nonexistent or not a scalar!");
			CheckException((dfNode["z+"] && dfNode["z+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'z+' field is nonexistent or not a scalar!");
			CheckException((dfNode["z-"] && dfNode["z+"].IsScalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'z-' field is nonexistent or not a scalar!");
			CheckException(std::filesystem::exists(dfNode["x+"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'x+' field refers to a nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["x-"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'x-' field refers to a nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["y+"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'y+' field refers to a nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["y-"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'y-' field refers to a nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["z+"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'z+' field refers to a nonexistent file!");
			CheckException(std::filesystem::exists(dfNode["z-"].Scalar()), Exception::GetExceptionCodeFromMeaning("InvalidYAML"), "While parsing skybox definition: 'z-' field refers to a nonexistent file!");

			//Create and compile skybox cubemap
			Cubemap* cube = new Cubemap(std::vector<std::string> {dfNode["x+"].Scalar(), dfNode["x-"].Scalar(), dfNode["y+"].Scalar(), dfNode["y-"].Scalar(), dfNode["z+"].Scalar(), dfNode["z-"].Scalar()});
			cube->CompileSync();

			//Create skybox
			std::shared_ptr<Skybox> asset = std::make_shared<Skybox>(cube);

			//Add asset to cache and return handle
			this->assetCache.insert_or_assign(definitionPath, std::weak_ptr<Skybox> {asset});
			return AssetHandle<Skybox>(definitionPath, asset);
		});
	}

	std::future<AssetHandle<Mesh>> AssetManager::LoadMesh(std::string location) {
		return Engine::GetInstance()->GetThreadPool()->enqueue([this, location]() {
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(location) && this->assetCache[location].lock()->GetType().compare("MESH") == 0) return AssetHandle<Mesh>(location, std::dynamic_pointer_cast<Mesh>(this->assetCache[location].lock()));

			//Split location parameter
			std::size_t pos = location.find(':');
			std::string model = location.substr(0, pos);
			std::string mesh = location.substr(pos + 1, location.size());

			//Confirm existence of model file
			CheckException(std::filesystem::exists(model), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load mesh from nonexistent model file!");

			//Load model
			Model mod {model};

			//Check that mesh is in model
			std::vector<std::string> meshList = mod.ListMeshes();
			CheckException(std::find(meshList.cbegin(), meshList.cend(), mesh) != meshList.cend(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "While loading mesh from model: Mesh does not exist in loaded model!");

			//Extract mesh, compile it, and add it to asset cache
			std::shared_ptr<Mesh> asset;
			asset.reset(mod.ExtractMesh(mesh));
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
