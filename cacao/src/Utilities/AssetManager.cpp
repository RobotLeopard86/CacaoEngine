#include "Utilities/AssetManager.hpp"

#include "Core/Engine.hpp"
#include "3D/Model.hpp"

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

	std::future<Asset<Shader>> AssetManager::LoadShader(std::string vertexPath, std::string fragmentPath, ShaderSpec specification) {
		return Engine::GetInstance()->GetThreadPool().submit_task([vertexPath, fragmentPath, specification](){
			Shader* shader = new Shader(vertexPath, fragmentPath, specification);
			shader->Compile().wait();
			return Asset<Shader>(shader);
		});
	}

	std::future<Asset<Texture2D>> AssetManager::LoadTexture2D(std::string path) {
		return Engine::GetInstance()->GetThreadPool().submit_task([path](){
			Texture2D* tex = new Texture2D(path);
			tex->Compile().wait();
			return Asset<Texture2D>(tex);
		});
	}

	std::future<Asset<Cubemap>> AssetManager::LoadCubemap(std::vector<std::string> texturePaths) {
		return Engine::GetInstance()->GetThreadPool().submit_task([texturePaths](){
			Cubemap* tex = new Cubemap(texturePaths);
			tex->Compile().wait();
			return Asset<Cubemap>(tex);
		});
	}

	std::future<Asset<Skybox>> AssetManager::LoadSkybox(std::vector<std::string> texturePaths) {
		return Engine::GetInstance()->GetThreadPool().submit_task([texturePaths](){
			Cubemap* tex = new Cubemap(texturePaths);
			tex->Compile().wait();
			Skybox* skybox = new Skybox(tex);
			return Asset<Skybox>(skybox);
		});
	}

	std::future<Asset<Mesh>> AssetManager::LoadMesh(std::string modelPath, std::string meshID) {
		return Engine::GetInstance()->GetThreadPool().submit_task([modelPath, meshID](){
			Model model(modelPath);
			Mesh* mesh = model.ExtractMesh(meshID);
			mesh->Compile().wait();
			return Asset<Mesh>(mesh);
		});
	}
}
