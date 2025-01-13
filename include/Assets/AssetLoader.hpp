#pragma once

#include "Core/DllHelper.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "3D/Mesh.hpp"
#include "UI/Font.hpp"
#include "Audio/Sound.hpp"

namespace Cacao {
	class CACAO_API AssetLoader {
	  public:
		virtual ~AssetLoader() = default;

		/**
		 * @brief Load a shader from the provided asset ID
		 *
		 * @param id The ID of the shader to load
		 *
		 * @return Managed smart pointer to the shader
		 */
		virtual std::shared_ptr<Shader> GetShader(std::string id) = 0;

		/**
		 * @brief Load a mesh from the provided asset ID
		 *
		 * @param id The ID of the mesh to load
		 *
		 * @return Managed smart pointer to the mesh
		 */
		virtual std::shared_ptr<Mesh> GetMesh(std::string id) = 0;

		/**
		 * @brief Load a 2D texture from the provided asset ID
		 *
		 * @param id The ID of the 2D texture to load
		 *
		 * @return Managed smart pointer to the 2D texture
		 */
		virtual std::shared_ptr<Texture2D> GetTexture2D(std::string id) = 0;

		/**
		 * @brief Load a cubemap from the provided asset ID
		 *
		 * @param id The ID of the cubemap to load
		 *
		 * @return Managed smart pointer to the cubemap
		 */
		virtual std::shared_ptr<Cubemap> GetCubemap(std::string id) = 0;

		/**
		 * @brief Load a sound from the provided asset ID
		 *
		 * @param id The ID of the sound to load
		 *
		 * @return Managed smart pointer to the sound
		 */
		virtual std::shared_ptr<Sound> GetSound(std::string id) = 0;

		/**
		 * @brief Load a font from the provided asset ID
		 *
		 * @param id The ID of the font to load
		 *
		 * @return Managed smart pointer to the font
		 */
		virtual std::shared_ptr<Font> GetFont(std::string id) = 0;
	};
}