#pragma once

#include "libjaguar/Document.hpp"
#include "libjaguar/Traits.hpp"

#include <string>
#include <cstdint>
#include <variant>

#include "DllHelper.hpp"

namespace libcacaoasset {
	/**
	 * @brief Representation of any type of resource within an asset pack
	 */
	struct CACAO_API Resource {
		/**
		 * @brief Enumeration of all resource types
		 */
		enum class Type : uint8_t {
			Blob = 0,
			Shader = 1,
			Material = 2,
			Tex2D = 3,
			Cubemap = 4,
			Audio = 5,
			Font = 6,
			Model = 7,
			Mesh = 8,///<This type is used @b only for resource address validation mode selection; using it with other parts of this library will error!
			World = 9///<This type is used @b only for resource address validation mode selection; using it with other parts of this library will error!
		};

		std::string id;					 ///<Asset ID or resource path (not valid address)
		std::vector<unsigned char> bytes;///<Asset data encoded as bytes; use @c Decode* functions to parse
		Type type;						 ///<Resource type
	};

	/**
	 * @brief Representation of a world resource
	 */
	struct CACAO_API World {
		std::string skybox;						  ///<Skybox resource address
		libjaguar::Vector<float, 3> initialCamPos;///<Initial camera position
		libjaguar::Vector<float, 3> initialCamRot;///<Initial camera rotation

		///@brief Type for components on actors
		struct CACAO_API Component {
			std::string typeID;					  ///<Component type ID
			std::vector<unsigned char> reflection;///<Jaguar-encoded component reflection data (for use with Astra)
		};

		///@brief Type for actors in the world
		struct CACAO_API Actor {
			std::array<unsigned char, 16> guid;		 ///<Actor GUID
			std::array<unsigned char, 16> parentGUID;///<GUID of parent actor or all zeroes if this is a top-level actor
			std::string name;						 ///<Human-friendly actor name
			libjaguar::Vector<float, 3> initialPos;	 ///<Initial position
			libjaguar::Vector<float, 3> initialRot;	 ///<Initial rotation
			libjaguar::Vector<float, 3> initialScale;///<Initial scale
			std::vector<Component> components;		 ///<Components mounted on this actor initially
		};
		std::vector<Actor> actors;///<All actors in the world
	};

	/**
	 * @brief Representation of a shader resource
	 */
	struct CACAO_API Shader {
		std::vector<unsigned char> irCode;///<Compiled Slang IR
		struct CACAO_API Descriptor {
			/**
			 * @brief The type of object a shader is for
			 */
			enum class Type : uint8_t {
				Opaque3D = 0xA,	  ///<3D opaque geometry in the world
				NonOpaque3D = 0xD,///<3D potentially non-opaque geometry in the world
				Surface2D = 0xE	  ///<2D object rendered on a surface
			} type;				  ///<Type of this shader

			/**
			 * @brief Bits of used input data fields
			 */
			enum class InputBits : uint8_t {
				Position = 1 << 0, ///<Shader uses position in local space
				TexCoords = 1 << 1,///<Shader uses texture coordinates
				Normal = 1 << 2,   ///<Shader uses surface normal vector
				Tangent = 1 << 3,  ///<Shader uses surface tangent vector
				Bitangent = 1 << 4,///<Shader uses surface bitangent vector
				Transform = 1 << 5,///<Shader uses transform matrix
				Globals = 1 << 6,  ///<Shader uses globals matrix
				RESERVED = 1 << 7, ///<Reserved for later use
			};

			InputBits inputs;///<Input values used by the shader, represented as a bitmask of InputBits

			/**
			 * @brief Info about a uniform parameter
			 */
			struct CACAO_API UniformParameter {
				enum class DataType : uint8_t {
					Int = 0xD1,		///<32-bit signed integer
					UInt = 0xE1,	///<32-bit unsigned integer
					Float = 0xF1,	///<Single-precision floating point number (32-bit)
					Bool = 0xB1,	///<Boolean
					Int2 = 0xD2,	///<2-component int vector
					Int3 = 0xD3,	///<3-component int vector
					Int4 = 0xD4,	///<4-component int vector
					UInt2 = 0xE2,	///<2-component unsigned int vector
					UInt3 = 0xE3,	///<3-component unsigned int vector
					UInt4 = 0xE4,	///<4-component unsigned int vector
					Float2 = 0xF2,	///<2-component float vector
					Float3 = 0xF3,	///<3-component float vector
					Float4 = 0xF4,	///<4-component float vector
					Float2x2 = 0x22,///<2x2 matrix of floats
					Float3x3 = 0x33,///<3x3 matrix of floats
					Float4x4 = 0x44,///<4x4 matrix of floats
				} type;				///<Type of stored data

				std::string name;		  ///<Parameter name
				unsigned int bufferOffset;///<Offset into the parameters constant buffer at which to write the data
			};

			/**
			 * @brief Information about a texture parameter
			 */
			struct CACAO_API TextureParameter {
				std::string name;	 ///<Parameter name
				unsigned int binding;///<Descriptor binding index
				bool isCubemap;		 ///<If the texture is a Cubemap or a Tex2D
			};

			std::vector<UniformParameter> uniformParams;///<Uniform parameters list
			std::vector<TextureParameter> texParams;	///<Texture parameters list
		} descriptor;									///<Shader information descriptor
	};

	/**
	 * @brief Representation of a cubemap resource
	 */
	struct CACAO_API Cubemap {
		std::vector<unsigned char> left;  ///<Image data for left face
		std::vector<unsigned char> right; ///<Image data for right face
		std::vector<unsigned char> top;	  ///<Image data for top face
		std::vector<unsigned char> bottom;///<Image data for bottom face
		std::vector<unsigned char> front; ///<Image data for front face
		std::vector<unsigned char> back;  ///<Image data for back face
	};

	/**
	 * @brief Representation of a material resource
	 */
	struct CACAO_API Material {
		/**
		 * @brief A special object to refer to textures in material data storage
		 */
		struct CACAO_API TexRef {
			std::string address;///<The resource address of the target texture
			bool isCubemap;		///<If the address refers to a cubemap or not
		};

		using Storage = std::variant<int, unsigned int, float, bool, TexRef, libjaguar::Vector<float, 2>, libjaguar::Vector<float, 3>, libjaguar::Vector<float, 4>,
			libjaguar::Vector<int, 2>, libjaguar::Vector<int, 3>, libjaguar::Vector<int, 4>, libjaguar::Vector<unsigned int, 2>, libjaguar::Vector<unsigned int, 3>,
			libjaguar::Vector<unsigned int, 4>, libjaguar::Matrix<float, 2, 2>, libjaguar::Matrix<float, 3, 3>, libjaguar::Matrix<float, 4, 4>>;

		struct CACAO_API Param {
			std::string target;///<The shader parameter that receives the stored data
			Storage storage;   ///<The data storage for the parameter value
		};

		std::string shaderAddress;	  ///<Resource address of the base shader
		std::vector<Param> parameters;///<Data to apply to the shader
	};

	/**
	 * @brief Interface for lazy access to an asset pack file
	 */
	class CACAO_API AssetPack {
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
		 * @brief Create an empty asset pack`
		 *
		 * @return The new pack
		 */
		static AssetPack CreateEmpty();

		/**
		 * @brief Access a resource
		 *
		 * @param address The resource's address
		 *
		 * @throws std::runtime_error If the resource address provided is invalid
		 * @throws std::runtime_error If the resource does not exist in the pack
		 *
		 * @return The resource object
		 */
		Resource GetResource(const std::string& address);

		/**
		 * @brief Get a list of resources in the pack
		 *
		 * @return A list of all resource addresses
		 */
		std::vector<std::string> ListResources();

		/**
		 * @brief Get a list of resources of a certain type in the pack
		 *
		 * @return A list of all matchingresource addresses
		 *
		 * @throws std::runtime_error If the provided type is a type of resource that cannot be embedded in a pack
		 */
		std::vector<std::string> ListResourcesOfType(Resource::Type type);

		/**
		 * @brief Insert a resource into the pack, replacing any previous resources at its address
		 *
		 * @param address The resource's address
		 * @param resource The Resource object
		 *
		 * @throws std::runtime_error If the resource address provided is invalid
		 */
		void PutResource(const std::string& address, Resource&& resource);

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

		void RegisterResourceType();
		static Resource _DecResource(libjaguar::Document::ObjReader& rd);
		static void _EncResource(const Resource& r, libjaguar::Document::ObjWriter& wr);
	};

	/**
	 * @brief Validate a resource address
	 *
	 * @param address The address to validate
	 * @param type The resource type to use for checking (since they have different requirements)
	 *
	 * @return If the address is valid
	 */
	CACAO_API bool ValidateResourceAddress(const std::string& address, Resource::Type type);

	/**
	 * @brief Decode a world object from the stream data
	 *
	 * @param stream The stream to read data from
	 *
	 * @return The resulting world object
	 *
	 * @throws std::runtime_error If the stream pointer is invalid
	 * @throws std::runtime_error If the stream is not a valid world
	 * @throws std::runtime_error If decoding fails
	 */
	CACAO_API World DecodeWorld(std::istream* stream);

	/**
	 * @brief Export a world object to a stream
	 *
	 * @param world The World to export
	 * @param stream The stream to write to
	 *
	 * @throws std::runtime_error If the stream pointer is invalid
	 * @throws std::runtime_error If encoding fails
	 */
	CACAO_API void EncodeWorld(const World& world, std::ostream* stream);

	/**
	 * @brief Decode a shader object from the stream data
	 *
	 * @param stream The stream to read data from
	 *
	 * @return The resulting shader object
	 *
	 * @throws std::runtime_error If the stream pointer is invalid
	 * @throws std::runtime_error If the stream is not a valid shader
	 * @throws std::runtime_error If decoding fails
	 */
	CACAO_API Shader DecodeShader(std::istream* stream);

	/**
	 * @brief Export a shader object to a stream
	 *
	 * @param shader The Shader to export
	 * @param stream The stream to write to
	 *
	 * @throws std::runtime_error If the stream pointer is invalid
	 * @throws std::runtime_error If encoding fails
	 */
	CACAO_API void EncodeShader(const Shader& shader, std::ostream* stream);

	/**
	 * @brief Decode a material object from the stream data
	 *
	 * @param stream The stream to read data from
	 *
	 * @return The resulting material object
	 *
	 * @throws std::runtime_error If the stream pointer is invalid
	 * @throws std::runtime_error If the stream is not a valid material
	 * @throws std::runtime_error If decoding fails
	 */
	CACAO_API Material DecodeMaterial(std::istream* stream);

	/**
	 * @brief Export a material object to a stream
	 *
	 * @param material The Material to export
	 * @param stream The stream to write to
	 *
	 * @throws std::runtime_error If the stream pointer is invalid
	 * @throws std::runtime_error If encoding fails
	 */
	CACAO_API void EncodeMaterial(const Material& material, std::ostream* stream);

	/**
	 * @brief Decode a cubemap object from the stream data
	 *
	 * @param stream The stream to read data from
	 *
	 * @return The resulting cubemap object
	 *
	 * @throws std::runtime_error If the stream pointer is invalid
	 * @throws std::runtime_error If the stream is not a valid cubemap
	 * @throws std::runtime_error If decoding fails
	 */
	CACAO_API Cubemap DecodeCubemap(std::istream* stream);

	/**
	 * @brief Export a cubemap object to a stream
	 *
	 * @param cubemap The Cubemap to export
	 * @param stream The stream to write to
	 *
	 * @throws std::runtime_error If the stream pointer is invalid
	 * @throws std::runtime_error If encoding fails
	 */
	CACAO_API void EncodeCubemap(const Cubemap& cubemap, std::ostream* stream);

	///@cond
	CACAO_API constexpr Shader::Descriptor::InputBits operator|(Shader::Descriptor::InputBits a, Shader::Descriptor::InputBits b) noexcept {
		using U = std::underlying_type_t<Shader::Descriptor::InputBits>;
		return static_cast<Shader::Descriptor::InputBits>(
			static_cast<U>(a) | static_cast<U>(b));
	}

	CACAO_API constexpr Shader::Descriptor::InputBits operator&(Shader::Descriptor::InputBits a, Shader::Descriptor::InputBits b) noexcept {
		using U = std::underlying_type_t<Shader::Descriptor::InputBits>;
		return static_cast<Shader::Descriptor::InputBits>(
			static_cast<U>(a) & static_cast<U>(b));
	}

	CACAO_API constexpr Shader::Descriptor::InputBits operator~(Shader::Descriptor::InputBits a) noexcept {
		using U = std::underlying_type_t<Shader::Descriptor::InputBits>;
		return static_cast<Shader::Descriptor::InputBits>(
			static_cast<U>(~static_cast<U>(a)));
	}

	CACAO_API constexpr Shader::Descriptor::InputBits& operator|=(Shader::Descriptor::InputBits& a, Shader::Descriptor::InputBits b) noexcept {
		return a = a | b;
	}

	CACAO_API constexpr Shader::Descriptor::InputBits& operator&=(Shader::Descriptor::InputBits& a, Shader::Descriptor::InputBits b) noexcept {
		return a = a & b;
	}
	///@endcond
}