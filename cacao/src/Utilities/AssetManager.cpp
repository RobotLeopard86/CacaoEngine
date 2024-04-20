#include "Utilities/AssetManager.hpp"

#include "Core/Engine.hpp"
#include "3D/Model.hpp"

#include "yaml-cpp/yaml.h"

//Error checker
#define checkErr(condition, failMsg, retType) if(!condition) {\
				Logging::EngineLog(failMsg, LogLevel::Error);\
				return AssetHandle<retType>{};\
			}

namespace Cacao {
	//Required static variable initialization
	AssetManager* AssetManager::instance = nullptr;
	bool AssetManager::instanceExists = false;

	//Singleton accessor
	AssetManager* AssetManager::GetInstance() {
		//Do we have an instance yet?
		if(!instanceExists || instance == NULL){
			//Create instance
			instance = new AssetManager();
			instanceExists = true;
		}

		return instance;
	}

	void _AH::_AM_Uncache_AHID(std::string id){
		AssetManager::GetInstance()->UncacheAsset(id);
	}

	std::future<AssetHandle<Shader>> AssetManager::LoadShader(std::string definitionPath) {
		return Engine::GetInstance()->GetThreadPool().submit_task([this, definitionPath](){
			checkErr(std::filesystem::exists(definitionPath), "Cannot load shader from nonexistent definition file!", Shader)

			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(definitionPath) && this->assetCache[definitionPath].lock()->GetType().compare("SHADER") == 0) return AssetHandle<Shader>(definitionPath, std::dynamic_pointer_cast<Shader>(this->assetCache[definitionPath].lock()));

			//Load and validate definition file
			YAML::Node dfNode = YAML::LoadFile(definitionPath);
			checkErr(dfNode.IsMap(), "While parsing shader definition: File is not a map!", Shader)
			checkErr(dfNode["vertex"].IsScalar(), "While parsing shader definition: File does not contain required 'vertex' attribute or it is not a scalar!", Shader)
			checkErr(dfNode["fragment"].IsScalar(), "While parsing shader definition: File does not contain required 'fragment' attribute or it is not a scalar!", Shader)
			checkErr(std::filesystem::exists(dfNode["vertex"].Scalar()), "While parsing shader definition: 'vertex' attribute references nonexistent file!", Shader)
			checkErr(std::filesystem::exists(dfNode["fragment"].Scalar()), "While parsing shader definition: 'fragment' attribute references nonexistent file!", Shader)
			checkErr(dfNode["spec"] && dfNode["spec"].IsSequence(), "While parsing shader definition: 'spec' attribute is not a sequence!", Shader)
			
			//Validate and try to build spec
			int specEntryCounter = 1;
			ShaderSpec spec;
			constexpr std::array validTypes{
				"bool", "int", "int64", "uint", "uint64", "double", "float", "texture"
			};
			
			for(auto node : dfNode["spec"]){
				//Convience string generator function
				auto genErr = [specEntryCounter](std::string err){
					std::stringstream stream;
					stream << "While parsing shader definition spec, entry #" << specEntryCounter << ": " << err;
					return stream.str();
				};

				//Validate
				checkErr(node.IsMap(), genErr("Spec entry is not a map!"), Shader)
				checkErr(node["name"].IsScalar(), genErr("'name' attribute is not a scalar!"), Shader)
				checkErr(node["sizex"], genErr("'sizex' attribute does not exist!"), Shader)
				checkErr(node["sizex"].IsScalar() && node["sizex"].as<int>(INT_MIN) != INT_MIN, genErr("'sizex' attribute is not a number!"), Shader)
				checkErr((node["sizex"].as<int>(INT_MIN) > 0 && node["sizex"].as<int>(INT_MIN) < 5), genErr("'sizex' attribute is not a value from 1-4!"), Shader)
				checkErr(node["sizey"], genErr("'sizey' attribute does not exist!"), Shader)
				checkErr(node["sizey"].IsScalar() && node["sizey"].as<int>(INT_MIN) != INT_MIN, genErr("'sizey' attribute is not a number!"), Shader)
				checkErr((node["sizey"].as<int>(INT_MIN) > 0 && node["sizey"].as<int>(INT_MIN) < 5), genErr("'sizey' attribute is not a value from 1-4!"), Shader)
				checkErr(node["type"], genErr("'type' attribute does not exist!"), Shader)
				checkErr(node["type"].IsScalar() && std::find(validTypes.cbegin(), validTypes.cend(), node["type"].Scalar()) != validTypes.cend(), genErr("'type' attribute is not a valid type!"), Shader)
				
				//Build spec entry
				ShaderItemInfo inf;
				inf.entryName = node["name"].Scalar();
				inf.size = { node["sizex"].as<int>(INT_MIN), node["sizey"].as<int>(INT_MIN) };

				//Find index of value in valid type array
				int idx = 0;
				for(; idx < validTypes.size(); idx++) {
					if(node["type"].Scalar().compare(validTypes[idx]) == 0) break;
				}

				switch(idx){
				case 0: //bool
					inf.type = SpvType::Boolean;
					break;
				case 1: //int
					inf.type = SpvType::Int;
					break;
				case 2: //int64
					inf.type = SpvType::Int64;
					break;
				case 3: //uint
					inf.type = SpvType::UInt;
					break;
				case 4: //uint64
					inf.type = SpvType::UInt64;
					break;
				case 5: //float
					inf.type = SpvType::Float;
					break;
				case 6: //double
					inf.type = SpvType::Double;
					break;
				case 7: //image
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
			asset->Compile().wait();

			//Add asset to cache
			this->assetCache.insert_or_assign(definitionPath, std::weak_ptr<Shader>{asset});

			//Construct and return asset handle
			return AssetHandle<Shader>(definitionPath, asset);
		});
	}

	std::future<AssetHandle<Texture2D>> AssetManager::LoadTexture2D(std::string path) {
		return Engine::GetInstance()->GetThreadPool().submit_task([this, path](){
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(path) && this->assetCache[path].lock()->GetType().compare("2DTEX") == 0) return AssetHandle<Texture2D>(path, std::dynamic_pointer_cast<Texture2D>(this->assetCache[path].lock()));

			//Construct an asset, add it to cache, and return a handle
			std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>(path);
			tex->Compile().wait();
			this->assetCache.insert_or_assign(path, std::weak_ptr<Texture2D>{tex});
			return AssetHandle<Texture2D>(path, tex);
		});
	}

	std::future<AssetHandle<Cubemap>> AssetManager::LoadCubemap(std::string definitionPath) {
		return Engine::GetInstance()->GetThreadPool().submit_task([this, definitionPath](){
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(definitionPath) && this->assetCache[definitionPath].lock()->GetType().compare("CUBEMAP") == 0) return AssetHandle<Cubemap>(definitionPath, std::dynamic_pointer_cast<Cubemap>(this->assetCache[definitionPath].lock()));

			checkErr(std::filesystem::exists(definitionPath), "Cannot load cubemap from nonexistent definition file!", Cubemap)

			//Load and validate definition file
			YAML::Node dfNode = YAML::LoadFile(definitionPath);
			checkErr(dfNode.IsMap(), "While parsing cubemap definition: File is not a map!", Cubemap)
			checkErr((dfNode["x+"] && dfNode["x+"].IsScalar()), "While parsing cubemap definition: 'x+' field is nonexistent or not a scalar!", Cubemap)
			checkErr((dfNode["x-"] && dfNode["x+"].IsScalar()), "While parsing cubemap definition: 'x-' field is nonexistent or not a scalar!", Cubemap)
			checkErr((dfNode["y+"] && dfNode["y+"].IsScalar()), "While parsing cubemap definition: 'y+' field is nonexistent or not a scalar!", Cubemap)
			checkErr((dfNode["y-"] && dfNode["y+"].IsScalar()), "While parsing cubemap definition: 'y-' field is nonexistent or not a scalar!", Cubemap)
			checkErr((dfNode["z+"] && dfNode["z+"].IsScalar()), "While parsing cubemap definition: 'z+' field is nonexistent or not a scalar!", Cubemap)
			checkErr((dfNode["z-"] && dfNode["z+"].IsScalar()), "While parsing cubemap definition: 'z-' field is nonexistent or not a scalar!", Cubemap)
			checkErr(std::filesystem::exists(dfNode["x+"].Scalar()), "While parsing cubemap definition: 'x+' field refers to a nonexistent file!", Cubemap)
			checkErr(std::filesystem::exists(dfNode["x-"].Scalar()), "While parsing cubemap definition: 'x-' field refers to a nonexistent file!", Cubemap)
			checkErr(std::filesystem::exists(dfNode["y+"].Scalar()), "While parsing cubemap definition: 'y+' field refers to a nonexistent file!", Cubemap)
			checkErr(std::filesystem::exists(dfNode["y-"].Scalar()), "While parsing cubemap definition: 'y-' field refers to a nonexistent file!", Cubemap)
			checkErr(std::filesystem::exists(dfNode["z+"].Scalar()), "While parsing cubemap definition: 'z+' field refers to a nonexistent file!", Cubemap)
			checkErr(std::filesystem::exists(dfNode["z-"].Scalar()), "While parsing cubemap definition: 'z-' field refers to a nonexistent file!", Cubemap)
		
			//Create and compile cubemap
			std::shared_ptr<Cubemap> asset = std::make_shared<Cubemap>(std::vector<std::string>{ dfNode["x+"].Scalar(), dfNode["x-"].Scalar(), dfNode["y+"].Scalar(), dfNode["y-"].Scalar(), dfNode["z+"].Scalar(), dfNode["z-"].Scalar() });
			asset->Compile().wait();

			//Add asset to cache and return handle
			this->assetCache.insert_or_assign(definitionPath, std::weak_ptr<Cubemap>{asset});
			return AssetHandle<Cubemap>(definitionPath, asset);
		});
	}

	std::future<AssetHandle<Skybox>> AssetManager::LoadSkybox(std::string definitionPath) {
		return Engine::GetInstance()->GetThreadPool().submit_task([this, definitionPath](){
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(definitionPath) && this->assetCache[definitionPath].lock()->GetType().compare("SKYBOX") == 0) return AssetHandle<Skybox>(definitionPath, std::dynamic_pointer_cast<Skybox>(this->assetCache[definitionPath].lock()));

			checkErr(std::filesystem::exists(definitionPath), "Cannot load skybox from nonexistent definition file!", Skybox)

			//Load and validate definition file
			YAML::Node dfNode = YAML::LoadFile(definitionPath);
			checkErr(dfNode.IsMap(), "While parsing skybox definition: File is not a map!", Skybox)
			checkErr((dfNode["x+"] && dfNode["x+"].IsScalar()), "While parsing skybox definition: 'x+' field is nonexistent or not a scalar!", Skybox)
			checkErr((dfNode["x-"] && dfNode["x+"].IsScalar()), "While parsing skybox definition: 'x-' field is nonexistent or not a scalar!", Skybox)
			checkErr((dfNode["y+"] && dfNode["y+"].IsScalar()), "While parsing skybox definition: 'y+' field is nonexistent or not a scalar!", Skybox)
			checkErr((dfNode["y-"] && dfNode["y+"].IsScalar()), "While parsing skybox definition: 'y-' field is nonexistent or not a scalar!", Skybox)
			checkErr((dfNode["z+"] && dfNode["z+"].IsScalar()), "While parsing skybox definition: 'z+' field is nonexistent or not a scalar!", Skybox)
			checkErr((dfNode["z-"] && dfNode["z+"].IsScalar()), "While parsing skybox definition: 'z-' field is nonexistent or not a scalar!", Skybox)
			checkErr(std::filesystem::exists(dfNode["x+"].Scalar()), "While parsing skybox definition: 'x+' field refers to a nonexistent file!", Skybox)
			checkErr(std::filesystem::exists(dfNode["x-"].Scalar()), "While parsing skybox definition: 'x-' field refers to a nonexistent file!", Skybox)
			checkErr(std::filesystem::exists(dfNode["y+"].Scalar()), "While parsing skybox definition: 'y+' field refers to a nonexistent file!", Skybox)
			checkErr(std::filesystem::exists(dfNode["y-"].Scalar()), "While parsing skybox definition: 'y-' field refers to a nonexistent file!", Skybox)
			checkErr(std::filesystem::exists(dfNode["z+"].Scalar()), "While parsing skybox definition: 'z+' field refers to a nonexistent file!", Skybox)
			checkErr(std::filesystem::exists(dfNode["z-"].Scalar()), "While parsing skybox definition: 'z-' field refers to a nonexistent file!", Skybox)
		
			//Create and compile skybox texture
			Cubemap* cube = new Cubemap(std::vector<std::string>{ dfNode["x+"].Scalar(), dfNode["x-"].Scalar(), dfNode["y+"].Scalar(), dfNode["y-"].Scalar(), dfNode["z+"].Scalar(), dfNode["z-"].Scalar() });;
			cube->Compile();

			//Create skybox asset
			std::shared_ptr<Skybox> asset = std::make_shared<Skybox>(cube);

			//Add asset to cache and return handle
			this->assetCache.insert_or_assign(definitionPath, std::weak_ptr<Skybox>{asset});
			return AssetHandle<Skybox>(definitionPath, asset);
		});
	}

	std::future<AssetHandle<Mesh>> AssetManager::LoadMesh(std::string location) {
		return Engine::GetInstance()->GetThreadPool().submit_task([this, location](){
			//First check if asset is already cached and return the cached one if so
			if(this->assetCache.contains(location) && this->assetCache[location].lock()->GetType().compare("MESH") == 0) return AssetHandle<Mesh>(location, std::dynamic_pointer_cast<Mesh>(this->assetCache[location].lock()));

			//Split location parameter
			size_t pos = location.find(':');
			std::string model = location.substr(0, pos);
			std::string mesh = location.substr(pos + 1, location.size());

			//Confirm existence of model file
			checkErr(std::filesystem::exists(model), "While parsing mesh location: Model file does not exist!", Mesh)

			//Load model
			Model mod{model};
			
			//Check that mesh is in model
			std::vector<std::string> meshList = mod.ListMeshes();
			checkErr((std::find(meshList.cbegin(), meshList.cend(), mesh) != meshList.cend()), "While loading mesh from model: Mesh does not exist in loaded model!", Mesh)

			//Extract mesh, compile it, and add it to asset cache
			std::shared_ptr<Mesh> asset;
			asset.reset(mod.ExtractMesh(mesh));
			asset->Compile().wait();
			this->assetCache.insert_or_assign(location, std::weak_ptr<Mesh>{asset});

			//Return asset handle
			return AssetHandle<Mesh>(location, asset);
		});
	}
}
