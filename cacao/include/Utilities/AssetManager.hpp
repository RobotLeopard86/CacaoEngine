#pragma once

#include "3D/Mesh.hpp"
#include "3D/Skybox.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "Graphics/Textures/Texture2D.hpp"

#include <future>

namespace Cacao {
	//Manager for an asset type
	template<typename T>
	class Asset {
	public:
		//Give an asset to the manager
		//The manager will now own the object and manage its lifetime
		Asset(T* subject) {
			managed = subject;
		}
		~Asset() {
			if(managed != NULL) delete managed;
		}
		//Prevent copying
		Asset(Asset&) = delete;
		Asset& operator=(const Asset&) = delete;
		
		//Steal managed object from original when moved
		Asset(Asset&& other) 
			: managed(other.managed) {
			other.managed = NULL;
		}
		Asset& operator=(Asset&& other) {
			managed = other.managed;
			other.managed = NULL;
			return *this;
		};

		//Necessary default constructor
		//TO BE USED ONLY FOR DEFAULT CONSTRUCTION, DO NOT USE INTENTIONALLY
		Asset()
			: managed(NULL) {}

		//Acces the contained object
		T* operator->() { return managed; }
		T* operator()() { return managed; }
	private:
		T* managed;
	};

	//Game asset manager
	class AssetManager {
	public:
		//Get the instance or create one if it doesn't exist.
		static AssetManager* GetInstance();

		//Load a shader with paths to compiled SPIR-V shader files and a shader specification
		std::future<Asset<Shader>> LoadShader(std::string vertexPath, std::string fragmentPath, ShaderSpec specification);

		//Load a 2D texture from a file
		std::future<Asset<Texture2D>> LoadTexture2D(std::string path);
		//Load a cubemap from files
		std::future<Asset<Cubemap>> LoadCubemap(std::vector<std::string> texturePaths);

		//Load a skybox with a cubemap created from files
		std::future<Asset<Skybox>> LoadSkybox(std::vector<std::string> texturePaths);

		//Load a mesh from a model file
		std::future<Asset<Mesh>> LoadMesh(std::string modelPath, std::string meshID);
	private:
		//Singleton members
		static AssetManager* instance;
		static bool instanceExists;

		AssetManager() {}
	};
}
