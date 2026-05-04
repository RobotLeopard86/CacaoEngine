#pragma once

#include "libjaguar/Document.hpp"

#include <string>
#include <cstdint>

namespace libcacaoasset {
	//This is all TODO but I had to make the file so

	/**
	 * @brief Representation of an asset in an asset pack
	 *
	 */
	struct Resource {
		/**
		 * @brief Enumeration of all resource types
		 */
		enum class Type : uint8_t {
			Blob = 0,
			Shader = 1,
			Material = 2,
			Cubemap = 3,
			Audio = 4,
			Font = 5,
			Model = 6
		};

		std::string id;					 ///<Asset ID or resource path
		std::vector<unsigned char> bytes;///<Asset data encoded as bytes; use @c Decode* functions to parse
		Type type;						 ///<Resource type
	};

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
		 * @throws std::runtime_error If the stream pointer is invalid
		 * @throws std::runtime_error If the stream is not an asset pack
		 * @throws std::runtime_error If decoding fails
		 */
		static AssetPack OpenFromStream(std::istream* stream);

		/**
		 * @brief Access a resource
		 *
		 * @param address The resource's address
		 *
		 * @throws std::runtime_error If the resource does not exist in the pack
		 *
		 * @return The resource object
		 */
		Resource GetResource(const std::string& address);

		/**
		 * @brief Write an asset pack to an output stream
		 *
		 * @param stream The stream to write the pack to
		 *
		 * @throws std::runtime_error If the stream pointer is invalid
		 * @throws std::runtime_error If encoding fails
		 */
		void Export(std::ostream* stream);

	  private:
		AssetPack() {}

		libjaguar::Document doc;

		static Resource _DecResource(libjaguar::Document::ObjReader& rd);
		static void _EncResource(const Resource& r, libjaguar::Document::ObjWriter& wr);
	};
}