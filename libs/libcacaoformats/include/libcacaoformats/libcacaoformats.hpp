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
		unsigned char* data;///<Decoded data
		struct Size {
			uint32_t x, y;
		} size;				 ///<Image dimensions
		uint8_t channelCount;///<Image channel count (1=Grayscale,3=RGB,4=RGBA)
	};

	///@brief Interface to the library
	class Instance {
		/**
		 * @brief Create a library instance that loads unpacked formats
		 *
		 * @note Available by compile-time selection
		 *
		 * @return Unpacked format loader
		 *
		 * @throw std::runtime_error If the library is not configured for unpacked format loading
		 */
		static Instance CreateUnpackedLoader();

		/**
		 * @brief Create a library instance that loads packed formats
		 *
		 * @note Available by compile-time selection
		 *
		 * @return Packed format loader
		 *
		 * @throw std::runtime_error If the library is not configured for packed format loading
		 */
		static Instance CreatePackedLoader();
	};
}