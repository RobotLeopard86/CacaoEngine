#pragma once

#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <array>

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
		///@brief Mini-encapsulation of size data
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

	///@brief List of codes identifying different packed formats
	enum class FormatCode {
		Cubemap = 0xC4,
		Shader = 0x1B,
		Material = 0x3E,
		AssetPack = 0xAA,
		World = 0x7A
	};

	///@brief Loaded structure of a generic packed file format
	struct PackedContainer {
		const FormatCode format;				 ///<Type of contents
		const std::array<uint8_t, 64> hash;		 ///<SHA-512 payload hash
		const uint16_t version;					 ///<File type version
		const std::vector<unsigned char> payload;///<Decompressed payload data
	};

	/**
	 * @brief Create a PackedContainer from a loaded buffer
	 *
	 * @param buffer A buffer containing the entire file contents of a packed file format
	 *
	 * @return PackedContainer object
	 *
	 * @throws std::runtime_error If the buffer is not a valid packed file
	 */
	PackedContainer MakePackedContainerFromBuffer(std::vector<unsigned char> buffer);
}