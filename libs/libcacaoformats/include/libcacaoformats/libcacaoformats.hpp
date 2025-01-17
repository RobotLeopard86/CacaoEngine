#pragma once

#include <vector>

namespace libcacaoformats {
	///@brief Loaded audio data and properties necessary to use it
	struct AudioBuffer {
		std::vector<short> data;///<PCM frames
		uint64_t sampleCount;	///<Number of audio samples
		uint32_t sampleRate;	///<Rate of samples per second
		uint8_t channelCount;	///<Audio channel count
	};

	///@brief Loaded image data and properties necessary to use it
	struct ImageBuffer {
		std::vector<unsigned char> data;///<Decoded data
		struct Size {
			uint32_t x, y;
		} size;				 ///<Image dimensions
		uint8_t channelCount;///<Image channel count (1=Grayscale,3=RGB,4=RGBA)
	};

	///@brief Loaded shader data
	struct Shader {
		std::vector<uint32_t> vertexSPV;  ///<Vertex shader code in SPIR-V
		std::vector<uint32_t> fragmentSPV;///<Fragment shader code in SPIR-V
	};

	///@brief Asset loader base class
	class Loader {
	  public:
		/**
		 * @brief Load a shader
		 *
		 * @param location The location to load the shader from. @see Loader implementation for formatting instructions.
		 *
		 * @return Shader object
		 */
		virtual Shader LoadShader(std::string location) = 0;

		/**
		 * @brief Load a texture
		 *
		 * @param location The location to load the texture from. @see Loader implementation for formatting instructions.
		 *
		 * @return ImageBuffer object
		 */
		virtual ImageBuffer LoadTex(std::string location) = 0;

		/**
		 * @brief Load a cubemap
		 *
		 * @param location The location to load the texture from. @see Loader implementation for formatting instructions.
		 *
		 * @return Array of ImageBuffer objects for each face (order: +X, -X, +Y, -Y, +Z, -Z)
		 */
		virtual std::array<ImageBuffer, 6> LoadCubemap(std::string location) = 0;

		/**
		 * @brief Load audio
		 *
		 * @param location The location to load the audio from. @see Loader implementation for formatting instructions.
		 *
		 * @return AudioBuffer object
		 */
		virtual AudioBuffer LoadAudio(std::string location) = 0;

		/**
		 * @brief Load a font face
		 *
		 * @param location The location to load the face from. @see Loader implementation for formatting instructions.
		 *
		 * @return Font data
		 */
		virtual std::vector<unsigned char> LoadFont(std::string location) = 0;
	};
}