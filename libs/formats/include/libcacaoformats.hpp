#pragma once

#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <array>
#include <variant>
#include <functional>
#include <istream>
#include <ostream>
#include <cstring>

#include "crossguid/guid.hpp"

namespace libcacaoformats {
	///@brief Two-component vector
	template<typename T>
	struct Vec2 {
		T x, y;
	};

	///@brief Three-component vector
	template<typename T>
	struct Vec3 {
		T x, y, z;
	};

	///@brief Four-component vector
	template<typename T>
	struct Vec4 {
		T x, y, z, w;
	};

	///@brief Matrix
	template<typename T, int M, int N>
	struct Matrix {
		std::array<std::array<T, M>, N> data;
		std::array<T, M>& operator[](int idx) {
			return data[idx];
		}
	};

	///@brief Decoded image data and properties necessary to use it
	struct ImageBuffer {
		std::vector<unsigned char> data;///<Decoded data
		Vec2<uint32_t> size;			///<Image dimensions
		uint8_t channelCount;			///<Image channel count (1=Grayscale,3=RGB,4=RGBA)
	};

	/**
	 * @brief Convenience function for decoding image data
	 *
	 * @details Supports JPEG, PNG, TGA, BMP, and HDR
	 *
	 * @param encoded The encoded image data to decode, provided via stream
	 *
	 * @return An ImageBuffer containing the decoded information
	 *
	 * @throws std::runtime_error If no data is provided, data is of an unsupported format, or the image decoding fails
	 */
	ImageBuffer DecodeImage(std::istream& encoded);

	/**
	 * @brief Convenience function for encoding ImageBuffer data to PNG format
	 *
	 * @param img The ImageBuffer to encode
	 * @param out A stream to output the resulting PNG-encoded data to
	 *
	 * @throws std::runtime_error If the image has zero dimensions or holds invalid data
	 */
	void EncodeImage(const ImageBuffer& img, std::ostream& out);

	///@brief Decoded shader data
	struct Shader {
		///@brief Type of shader code stored
		enum class CodeType {
			SPIRV,
			GLSL
		};

		///@brief SPIR-V code storage format
		using SPIRVCode = std::vector<uint32_t>;

		///@brief GLSL code storage format
		struct GLSLCode {
			std::string vertex;	 ///<Vertex shader code
			std::string fragment;///<Fragment shader code
		};

		CodeType type;						   ///<Format of stored shader code
		std::variant<SPIRVCode, GLSLCode> code;///<Stored code
	};

	///@brief List of codes identifying different packed formats
	enum class PackedFormat {
		Cubemap,
		Shader,
		Material,
		AssetPack,
		World
	};

	///@brief Encapsulation of data associated with an asset in an asset pack
	struct PackedAsset {
		///@brief Type of asset
		enum class Kind {
			Shader,
			Tex2D,
			Cubemap,
			Material,
			Sound,
			Font
		};
		Kind kind;						  ///<Type of asset contained
		std::vector<unsigned char> buffer;///<Asset file contents buffer
	};

	/**
	 * @brief Loaded structure of a generic packed file format
	 *
	 * @note You generally shouldn't use the direct constructor. For most use cases, a PackedEncoder should work instead.
	 */
	class PackedContainer {
	  public:
		const PackedFormat format;				 ///<Type of contents
		const uint16_t version;					 ///<File type version
		const std::vector<unsigned char> payload;///<Decompressed payload data

		/**
		 * @brief Create a PackedContainer from a stream
		 *
		 * @param stream A stream referencing the contents of a packed file format
		 *
		 * @return PackedContainer object
		 *
		 * @throws std::runtime_error If the stream is not valid, does not represent a valid packed file
		 */
		static PackedContainer FromStream(std::istream& stream);

		/**
		 * @brief Create a PackedContainer from a PackedAsset
		 *
		 * @param asset A packed asset from an asset pack
		 *
		 * @return PackedContainer object
		 *
		 * @throws std::runtime_error If the asset is of an unsupported type or the buffer is an invalid
		 */
		static PackedContainer FromAsset(const PackedAsset& asset);

		/**
		 * @brief Create a PackedContainer by manually specifying attributes
		 *
		 * @param format The format code of the container
		 * @param ver The version of the format
		 * @param data The uncompressed data to be stored
		 *
		 *
		 * @throws std::runtime_error If there is no data
		 */
		PackedContainer(PackedFormat format, uint16_t ver, std::vector<unsigned char>&& data);

		/**
		 * @brief Create a PackedContainer by manually specifying attributes, allowing a signed char
		 *
		 * @param format The format code of the container
		 * @param ver The version of the format
		 * @param data The uncompressed data to be stored, autoconverted to unsigned char
		 *
		 *
		 * @throws std::runtime_error If there is no data
		 */
		PackedContainer(PackedFormat format, uint16_t ver, std::vector<char>&& data);

		/**
		 * @brief Create an empty PackedContainer, only useful for default constructing if needed
		 *
		 * @warning This will produce a PackedContainer with no data, the version set to 0 and the type set to Shader. DO NOT USE THIS CONTAINER!
		 */
		PackedContainer()
		  : format(PackedFormat::Shader), version(0), payload() {}

		PackedContainer(const PackedContainer& o)
		  : format(o.format), version(o.version), payload(o.payload) {}
		PackedContainer(PackedContainer&& o)
		  : format(o.format), version(o.version), payload(o.payload) {}
		PackedContainer& operator=(const PackedContainer&) = delete;
		PackedContainer& operator=(PackedContainer&&) = delete;

		/**
		 * @brief Export a PackedContainer to a buffer
		 *
		 * @param stream A stream to output data to
		 *
		 * @throws std::runtime_error If the container has no data or data compression fails
		 */
		void ExportToStream(std::ostream& stream);
	};

	///@brief Reference path for shader and associated data
	struct Material {
		std::string shader;///<Path to reference shader by

		///@brief Simple struct to represent a texture asset path and indicate if it's a cubemap
		struct TextureRef {
			std::string path;///<Asset path
			bool isCubemap;	 ///<Is this a cubemap (true) or a 2D texture (false)?
		};

		///@brief Shorthand for container of possible data types
		using ValueContainer = std::variant<int, unsigned int, float,
			Vec2<int>, Vec3<int>, Vec4<int>,
			Vec2<unsigned int>, Vec3<unsigned int>, Vec4<unsigned int>,
			Vec2<float>, Vec3<float>, Vec4<float>,
			Matrix<float, 2, 2>, Matrix<float, 2, 3>, Matrix<float, 2, 4>,
			Matrix<float, 3, 2>, Matrix<float, 3, 3>, Matrix<float, 3, 4>,
			Matrix<float, 4, 2>, Matrix<float, 4, 3>, Matrix<float, 4, 4>,
			TextureRef>;
		std::map<std::string, ValueContainer> keys;///<Data associated with shader
	};

	///@brief World data, encapsulating list of assets and components used as well as initial world state
	struct World {
		std::string skyboxRef;	  ///<Skybox reference path
		Vec3<float> initialCamPos;///<Initial camera position
		Vec3<float> initialCamRot;///<Initial camera rotation

		///@brief Type for components on entities
		struct Component {
			std::string typeID;	   ///<ID of component type to instantiate
			std::string reflection;///<YAML-encoded component reflection data (for use with Silica)
		};

		///@brief Type for entities in the world
		struct Entity {
			xg::Guid guid;					  ///<Entity GUID
			xg::Guid parentGUID;			  ///<GUID of parent entity or all zeroes if this is a top-level entity
			std::string name;				  ///<Human-friendly entity name
			Vec3<float> initialPos;			  ///<Initial position
			Vec3<float> initialRot;			  ///<Initial rotation
			Vec3<float> initialScale;		  ///<Initial scale
			std::vector<Component> components;///<Components mounted on this entity initially
		};
		std::vector<Entity> entities;///<Entities in the world
	};

	///@brief Decoder for uncompressed packed format buffers
	class PackedDecoder {
	  public:
		/**
		 * @brief Extract and decode the images in a cubemap
		 *
		 * @param container The PackedContainer with the cubemap information
		 *
		 * @return Decoded cubemap faces in the order of +X face, -X face, +Y face, -Y face, +Z face, -Z face
		 *
		 * @throws std::runtime_error If the container does not hold a valid cubemap
		 */
		std::array<ImageBuffer, 6> DecodeCubemap(const PackedContainer& container);

		/**
		 * @brief Extract the SPIR-V code from a shader
		 *
		 * @param container The PackedContainer with the shader information
		 *
		 * @return Shader SPIR-V
		 *
		 * @throws std::runtime_error If the container does not hold a valid shader
		 */
		Shader DecodeShader(const PackedContainer& container);

		/**
		 * @brief Extract the data from a packed material
		 *
		 * @param container The PackedContainer with the material information
		 *
		 * @return Material object with shader reference string and data
		 *
		 * @throws std::runtime_error If the container does not hold a valid material
		 */
		Material DecodeMaterial(const PackedContainer& container);

		/**
		 * @brief Extract the data from a packed world
		 *
		 * @param container The PackedContainer with the world information
		 *
		 * @return World object containing the initial state of the world
		 *
		 * @throws std::runtime_error If the container does not hold a valid world
		 */
		World DecodeWorld(const PackedContainer& container);

		/**
		 * @brief Extract the files from an asset pack
		 *
		 * @param container The PackedContainer with the asset pack information
		 *
		 * @return Map of filenames to PackedAsset objects from the asset pack
		 *
		 * @throws std::runtime_error If the container does not hold a valid asset pack or the pack has no files
		 */
		std::map<std::string, PackedAsset> DecodeAssetPack(const PackedContainer& container);
	};

	///@brief Decoder for unpacked file formats
	class UnpackedDecoder {
	  public:
		/**
		 * @brief Extract and decode the images in a cubemap
		 *
		 * @param data A stream to load cubemap YAML data from
		 * @param loader A function to get streams from names referenced in the main cubemap file. Set the "badbit" flag on the stream before returning to indicate a failed load.
		 *
		 * @return Decoded cubemap faces in the order of +X face, -X face, +Y face, -Y face, +Z face, -Z face
		 *
		 * @throws std::runtime_error If the data does not represent a valid cubemap or the provided IO callback fails to load faces
		 */
		std::array<ImageBuffer, 6> DecodeCubemap(std::istream& data, std::function<std::istream(const std::string&)> loader);

		/**
		 * @brief Extract the data from an unpacked material
		 *
		 * @param data A stream to load material YAML data from
		 *
		 * @return Material object with shader reference string and data
		 *
		 * @throws std::runtime_error If the data does not represent a valid material
		 */
		Material DecodeMaterial(std::istream& data);

		/**
		 * @brief Extract the data from an unpacked world
		 *
		 * @param data A stream to load world YAML data from
		 *
		 * @return World object containing the initial state of the world
		 *
		 * @throws std::runtime_error If the data does not represent a valid world
		 */
		World DecodeWorld(std::istream& data);
	};

	///@brief Encoder for uncompressed packed format buffers
	class PackedEncoder {
	  public:
		/**
		 * @brief Encode a set of cubemap images faces into a packed cubemap
		 *
		 * @param cubemap A list of cubemap faces in the order of +X face, -X face, +Y face, -Y face, +Z face, -Z face
		 *
		 * @return A PackedContainer encapsulating the encoded cubemap data, with all faces encoded in PNG format
		 *
		 * @throws std::runtime_error If one of the faces holds invalid data or has zero dimensions
		 */
		PackedContainer EncodeCubemap(const std::array<ImageBuffer, 6>& cubemap);

		/**
		 * @brief Merge shader SPIR-V into a packed shader
		 *
		 * @param shader The vertex and fragment shader SPIR-V code
		 *
		 * @return A PackedContainer encapsulating the shader code
		 */
		PackedContainer EncodeShader(const Shader& shader);

		/**
		 * @brief Encode material data and a shader reference into a packed material
		 *
		 * @param mat The Material object with the data to encode
		 *
		 * @return A PackedContainer encapsulating the shader reference and material data
		 */
		PackedContainer EncodeMaterial(const Material& mat);

		/**
		 * @brief Encode world data into a packed format
		 *
		 * @param world The decoded World object
		 *
		 * @return A PackedContainer encapsulating the world data and initial state
		 */
		PackedContainer EncodeWorld(const World& world);

		/**
		 * @brief Combine asset pack files into a merged pack
		 *
		 * @param pack A map of filenames to PackedAsset objects from the asset pack
		 *
		 * @return A PackedContainer encapsulating the asset info and asset files
		 *
		 * @throws std::runtime_error If the provided pack data has no assets
		 */
		PackedContainer EncodeAssetPack(const std::map<std::string, PackedAsset>& pack);
	};

	///@brief Encoder for unpacked file formats
	class UnpackedEncoder {
	  public:
		/**
		 * @brief Encode a Material object to its unpacked format
		 *
		 * @param mat The Material object to encode
		 * @param out A stream to output material YAML data to
		 */
		void EncodeMaterial(const Material& mat, std::ostream& out);

		/**
		 * @brief Encode a World object to its unpacked format
		 *
		 * @param world The World object to encode
		 * @param out A stream to output material YAML data to
		 */
		void EncodeWorld(const World& world, std::ostream& out);
	};
}