#pragma once

#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <array>
#include <variant>
#include <functional>

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
	};

	///@brief Decoded audio data and properties necessary to use it
	struct AudioBuffer {
		std::vector<short> data;///<Audio data
		uint64_t sampleCount;	///<Number of audio samples
		uint32_t sampleRate;	///<Rate of samples per second
		uint8_t channelCount;	///<Audio channel count
	};

	/**
	 * @brief Convenience function for decoding audio data
	 *
	 * @details Supports MP3, WAV, Ogg Vorbis, and Ogg Opus
	 *
	 * @param encoded The encoded audio data to decode
	 *
	 * @return An AudioBuffer containing the decoded information
	 */
	AudioBuffer DecodeAudio(std::vector<unsigned char> encoded);

	///@brief Decoded image data and properties necessary to use it
	struct ImageBuffer {
		std::vector<unsigned char> data;///<Decoded data
		Vec2<uint32_t> size;			///<Image dimensions
		uint8_t channelCount;			///<Image channel count (1=Grayscale,3=RGB,4=RGBA)
	};

	/**
	 * @brief Convenience function for decoding image data
	 *
	 * @details Supports JPEG, PNG, TGA, BMP, GIF (non-animated), and HDR
	 *
	 * @param encoded The encoded image data to decode
	 *
	 * @return An ImageBuffer containing the decoded information
	 */
	ImageBuffer DecodeImage(std::vector<unsigned char> encoded);

	///@brief Decoded shader data
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

	/**
	 * @brief Loaded structure of a generic packed file format
	 *
	 * @note When loading a container, use FromBuffer. When creating a container for exporting, use FromData.
	 */
	struct PackedContainer {
	  public:
		const FormatCode format;				 ///<Type of contents
		const uint16_t version;					 ///<File type version
		const std::array<uint8_t, 64> hash;		 ///<SHA-512 uncompressed payload hash
		const std::vector<unsigned char> payload;///<Decompressed payload data

		/**
		 * @brief Create a PackedContainer from a loaded buffer
		 *
		 * @param buffer A buffer containing the entire file contents of a packed file format
		 *
		 * @return PackedContainer object
		 *
		 * @throws std::runtime_error If the buffer is not a valid packed file or the hash in the buffer does not match the payload
		 */
		static PackedContainer FromBuffer(std::vector<unsigned char> buffer);

		/**
		 * @brief Create a PackedContainer from its contents
		 *
		 * @param format The format code of the container
		 * @param ver The version of the format
		 * @param data The uncompressed data to be stored
		 *
		 * @return PackedContainer object
		 *
		 * @throws std::runtime_error If there is no data
		 */
		static PackedContainer FromData(FormatCode format, uint16_t ver, std::vector<unsigned char> data);

		/**
		 * @brief Export a PackedContainer to a buffer
		 *
		 * @return A buffer representing the container, which can be written to a file whole
		 *
		 * @throws std::runtime_error If the container has no data or data compression fails
		 */
		std::vector<unsigned char> ExportToBuffer();

	  private:
		PackedContainer(FormatCode, uint16_t, std::vector<unsigned char>);
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
		const Kind kind;						///<Type of asset contained
		const std::vector<unsigned char> buffer;///<Asset file contents buffer
	};

	///@brief Reference path for shader and associated data
	struct Material {
		std::string shader;///<Path to reference shader by

		///@brief Shorthand for container of possible data types
		using ValueContainer = std::variant<int, unsigned int, float,
			Vec2<float>, Vec3<float>, Vec4<float>,
			Matrix<float, 2, 2>, Matrix<float, 2, 3>, Matrix<float, 2, 4>,
			Matrix<float, 3, 2>, Matrix<float, 3, 3>, Matrix<float, 3, 4>,
			Matrix<float, 4, 2>, Matrix<float, 4, 3>, Matrix<float, 4, 4>,
			std::string>;
		std::map<std::string, ValueContainer> values;///<Data associated with shader
	};

	///@brief World data, encapsulating list of assets and components used as well as initial world state
	struct World {
		std::vector<std::string> imports;///<List of assets and components used in the world. Having something not present in this list used will make the world not load correctly in-engine.
		std::string skyboxRef;			 ///<Skybox reference path
		Vec3<float> initialCamPos;		 ///<Initial camera position
		Vec3<float> initialCamRot;		 ///<Initial camera rotation

		///@brief Type for components on entities
		struct Component {
			std::string typeID;		  ///<ID of component type to instantiate
			std::vector<uint8_t> data;///<Binary-encoded component reflection data (for use with https://github.com/chocolacula/easy_reflection_cpp)
		};

		///@brief Type for entities in the world
		struct Entity {
			std::array<uint8_t, 16> guid;	   ///<Entity GUID
			std::array<uint8_t, 16> parentGUID;///<GUID of parent entity or all zeroes if this is a top-level entity
			std::string name;				   ///<Human-friendly entity name
			Vec3<float> initialPos;			   ///<Initial position
			Vec3<float> initialRot;			   ///<Initial rotation
			Vec3<float> initialScale;		   ///<Initial scale
			std::vector<Component> components; ///<Components mounted on this entity initially
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
		 * @throws std::runtime_error If the container does not hold a valid asset pack
		 */
		std::map<std::string, PackedAsset> DecodeAssetPack(const PackedContainer& container);
	};

	///@brief Decoder for unpacked file formats
	class UnpackedDecoder {
	  public:
		/**
		 * @brief A shorthand type for a decoder IO callback
		 *
		 * @details This exists because libcacaoformats is a no-IO library. So, this exists to outsource IO to the consumer. The parameter is the file path needed.
		 */
		using IOCallback = std::function<std::vector<unsigned char>(const std::string&)>;

		/**
		 * @brief Extract and decode the images in a cubemap
		 *
		 * @param data A string containg YAML encoding the cubemap information
		 * @param callback The IO callback to load encoded image data
		 *
		 * @return Decoded cubemap faces in the order of +X face, -X face, +Y face, -Y face, +Z face, -Z face
		 *
		 * @throws std::runtime_error If the data does not represent a valid cubemap or the provided IO callback fails to load faces
		 */
		std::array<ImageBuffer, 6> DecodeCubemap(const std::string& data, IOCallback callback);

		/**
		 * @brief Extract the data from a packed material
		 *
		 * @param data A string containg YAML encoding the material information
		 *
		 * @return Material object with shader reference string and data
		 *
		 * @throws std::runtime_error If the data does not represent a valid material
		 */
		Material DecodeMaterial(const std::string& data);

		/**
		 * @brief Extract the data from a packed world
		 *
		 * @param data A string containg YAML encoding the world information
		 *
		 * @return World object containing the initial state of the world
		 *
		 * @throws std::runtime_error If the data does not represent a valid world
		 */
		World DecodeWorld(const std::string& data);
	};
}