#pragma once

#include <vector>
#include <array>

namespace libcacaoformats {
	namespace FormatCodes {
		constexpr uint8_t Cubemap = 0xC4;
		constexpr uint8_t Shader = 0x1B;
		constexpr uint8_t Material = 0x3E;
		constexpr uint8_t AssetPack = 0xAA;
		constexpr uint8_t World = 0x7A;
	}

	/**
	 * @brief Loaded packed format container
	 */
	struct PackedContainer {
		const uint8_t format;					 ///<Type as indictated by the memebers of FormatCodes
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
	PackedContainer PackedContainerFromBuffer(std::vector<unsigned char> buffer);
}