#pragma once

#include "libjaguar/Document.hpp"

#include <string>

namespace libcacaoasset {
	//This is all TODO but I had to make the file so

	/**
	 * @brief Interface for lazy access to an asset pack file
	 */
	class AssetPack {
	  public:
		/**
		 * @brief Open an asset pack file from on disk
		 *
		 * @param filePath The relative path (from the working directory) to the file to open
		 *
		 * @return The opened pack
		 *
		 * @throws std::runtime_error If the file does not exist or is not an asset pack
		 * @throws std::runtime_error If the pack is of an incompatible revision
		 * @throws std::runtime_error If decoding fails
		 */
		static AssetPack OpenFromFile(const std::string& filePath);

		/**
		 * @brief Open an asset pack file from an input stream
		 *
		 * @warning The new asset pack will <b>control the stream exclusively</b>, and it is not recoverable from the object. <b>Do not perform further operations on the stream after calling this!</b>
		 *
		 * @param stream The stream from which to load the pack
		 *
		 * @return The opened pack
		 *
		 * @throws std::runtime_error If the stream is not an asset pack
		 * @throws std::runtime_error If decoding fails
		 */
		static AssetPack OpenFromStream(std::istream* stream);

	  private:
		AssetPack() {}

		libjaguar::Document doc;
	};
}