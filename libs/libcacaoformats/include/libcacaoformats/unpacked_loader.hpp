#pragma once

#include "libcacaoformats/libcacaoformats.hpp"

namespace libcacaoformats {
	///@brief Loader for unpacked asset formats
	class UnpackedLoader final : public Loader {
	  public:
		/**
		 * @brief Load a shader
		 *
		 * @param location The location to load the shader from. @see Loader implementation for formatting instructions.
		 *
		 * @return Shader object
		 */
		Shader LoadShader(std::string location) override;

		/**
		 * @brief Load a texture
		 *
		 * @param location The location to load the texture from. @see Loader implementation for formatting instructions.
		 *
		 * @return ImageBuffer object
		 */
		ImageBuffer LoadTex(std::string location) override;

		/**
		 * @brief Load a cubemap
		 *
		 * @param location The location to load the texture from. @see Loader implementation for formatting instructions.
		 *
		 * @return Array of ImageBuffer objects for each face (order: +X, -X, +Y, -Y, +Z, -Z)
		 */
		std::array<ImageBuffer, 6> LoadCubemap(std::string location) override;

		/**
		 * @brief Load audio
		 *
		 * @param location The location to load the audio from. @see Loader implementation for formatting instructions.
		 *
		 * @return AudioBuffer object
		 */
		AudioBuffer LoadAudio(std::string location) override;

		/**
		 * @brief Load a font face
		 *
		 * @param location The location to load the face from. @see Loader implementation for formatting instructions.
		 *
		 * @return Font data
		 */
		std::vector<unsigned char> LoadFont(std::string location) override;
	};
}