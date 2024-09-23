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
	/**
	 * @brief Manages the loading of assets and the asset cache
	 */
	class AssetManager {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static AssetManager* GetInstance();

		/**
		 * @brief Load a shader from a shader definition file
		 * See the page "Definition Files" in the manual for their format
		 *
		 * @param definitionPath The path to a definition file
		 *
		 * @throws Exception If the definition file does not exist, has the wrong format, or fails to compile
		 * @see Shader::Compile
		 */
		std::future<AssetHandle<Shader>> LoadShader(std::string definitionPath);

		/**
		 * @brief Load a 2D texture from an image file
		 *
		 * @param path The path to an image
		 *
		 * @throws Exception If the image file does not exist or fails to compile
		 * @see Texture2D::Compile
		 */
		std::future<AssetHandle<Texture2D>> LoadTexture2D(std::string path);

		/**
		 * @brief Load a cubemap from a cubemap definition file
		 * See the page "Definition Files" in the manual for their format
		 *
		 * @param definitionPath The path to a definition file
		 *
		 * @throws Exception If the definition file does not exist, has the wrong format, or fails to compile
		 * @see Cubemap::Compile
		 */
		std::future<AssetHandle<Cubemap>> LoadCubemap(std::string definitionPath);

		/**
		 * @brief Load a skybox from a cubemap definition file
		 * See the page "Definition Files" in the manual for their format
		 *
		 * @param definitionPath The path to a definition file
		 *
		 * @throws Exception If the definition file does not exist, has the wrong format, or fails to compile
		 * @see Cubemap::Compile,LoadCubemap
		 */
		std::future<AssetHandle<Skybox>> LoadSkybox(std::string definitionPath);

		/**
		 * @brief Load a mesh from a given model file
		 * @details The input format is as follows: (model file path):(mesh name)
		 *
		 * @param location The location to retrieve the mesh from
		 *
		 * @throws Exception If the location parameter has the wrong format, the model file does not exist, the model fails to load, the model does not contain the requested mesh, or the mesh fails to compile
		 * @see Model::Model,Mesh::Compile
		 */
		std::future<AssetHandle<Mesh>> LoadMesh(std::string location);

		/**
		 * @brief Load a sound from a file
		 * @details Supports MP3, WAV, Ogg Vorbis, and Ogg Opus files
		 *
		 * @param path The path to a sound file
		 *
		 * @throws Exception If the sound file does not exist, fails to be read, or fails to compile (which may happen if the audio system is uninitialized)
		 * @see Sound::Compile
		 */
		std::future<AssetHandle<Sound>> LoadSound(std::string path);

		/**
		 * @brief Load a font face from a file
		 *
		 * @param path The path to a font file
		 *
		 * @throws Exception If the font file does not exist or the font face cannot be loaded from it
		 */
		std::future<AssetHandle<Font>> LoadFont(std::string path);

		/**
		 * @brief Remove an asset from the cache
		 *
		 * @note For use by the engine only
		 *
		 * @param assetID The ID of the asset to remove from the cache
		 *
		 * @throws Exception If the specified asset ID does not exist in the cache
		 */
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
