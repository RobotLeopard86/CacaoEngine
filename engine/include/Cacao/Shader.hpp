#pragma once

#include "DllHelper.hpp"
#include "Asset.hpp"

namespace Cacao {
	/**
	 * @brief Shader inputs and behavior description
	 */
	struct CACAO_API ShaderDescription {
		/**
		 * @brief The type of object a shader is for
		 */
		enum class Type {
			Opaque3D,	///<3D opaque geometry in the world
			NonOpaque3D,///<3D potentially non-opaque geometry in the world
			Surface2D	///<2D object rendered on a surface
		} type;			///<Type of this shader

		/**
		 * @brief Bits of used vertex input attributes
		 */
		enum class VertexInputBits : uint32_t {
			Position = 1 << 0, ///<Position in local space
			TexCoords = 1 << 1,///<Texture coordinates
			Normal = 1 << 2,   ///<Surface normal vector
			Tangent = 1 << 3,  ///<Surface tangent vector
			Bitangent = 1 << 4,///<Surface bitangent vector
		};

		VertexInputBits vertexInputs;///<Vertex input values used by the shader, represented as a bitmask of VertexInputBits

		bool transformUsed;///<If the shader uses the transformation matrix
		bool globalsUsed;  ///<If the shader uses the globals buffer

		/**
		 * @brief Info about a material parameter
		 */
		struct CACAO_API MaterialParamInfo {
			/**
			 * @brief Type of data
			 */
			enum class DataType {
				///@name Non-opaque types:
				///@{
				Int,	 ///<32-bit signed integer
				UInt,	 ///<32-bit unsigned integer
				Float,	 ///<Single-precision floating point number (32-bit)
				Bool,	 ///<Boolean
				Float2,	 ///<2-component float vector
				Float3,	 ///<3-component float vector
				Float4,	 ///<4-component float vector
				Int2,	 ///<2-component int vector
				Int3,	 ///<3-component int vector
				Int4,	 ///<4-component int vector
				UInt2,	 ///<2-component unsigned int vector
				UInt3,	 ///<3-component unsigned int vector
				UInt4,	 ///<4-component unsigned int vector
				Float2x2,///<2x2 matrix of floats
				Float3x3,///<3x3 matrix of floats
				Float4x4,///<4x4 matrix of floats
				///@}

				///@name Opaque types:
				///@{
				Tex2D,	///<Tex2D
				TexCube,///<Cubemap
						///@}
			} type;		///<Type of stored data

			std::string name;///<Parameter name

			unsigned int bufferOffset;///<<b>For non-opaque types:</b> offset into the parameters constant buffer
			unsigned int binding;	  ///<<b>For opaque types:</b> descriptor binding number
		};

		std::vector<MaterialParamInfo> materialParams;///<Material parameters list
	};

	//This is to allow for VertexInputBits to work as a bitmask
	///@cond
	constexpr ShaderDescription::VertexInputBits operator|(ShaderDescription::VertexInputBits a, ShaderDescription::VertexInputBits b) noexcept {
		using U = std::underlying_type_t<ShaderDescription::VertexInputBits>;
		return static_cast<ShaderDescription::VertexInputBits>(
			static_cast<U>(a) | static_cast<U>(b));
	}

	constexpr ShaderDescription::VertexInputBits operator&(ShaderDescription::VertexInputBits a, ShaderDescription::VertexInputBits b) noexcept {
		using U = std::underlying_type_t<ShaderDescription::VertexInputBits>;
		return static_cast<ShaderDescription::VertexInputBits>(
			static_cast<U>(a) & static_cast<U>(b));
	}

	constexpr ShaderDescription::VertexInputBits operator~(ShaderDescription::VertexInputBits a) noexcept {
		using U = std::underlying_type_t<ShaderDescription::VertexInputBits>;
		return static_cast<ShaderDescription::VertexInputBits>(
			static_cast<U>(~static_cast<U>(a)));
	}

	constexpr ShaderDescription::VertexInputBits& operator|=(ShaderDescription::VertexInputBits& a, ShaderDescription::VertexInputBits b) noexcept {
		return a = a | b;
	}

	constexpr ShaderDescription::VertexInputBits& operator&=(ShaderDescription::VertexInputBits& a, ShaderDescription::VertexInputBits b) noexcept {
		return a = a & b;
	}
	///@endcond

	/**
	 * @brief Asset type for GPU shaders
	 */
	class CACAO_API Shader : public Asset {
	  public:
		/**
		 * @brief Create a new shader from IR code
		 *
		 * @param shaderIR The Slang IR code to create the shader with
		 * @param desc Shader behavior and input information
		 * @param addr The resource address to associate with the shader
		 *
		 * @throws BadValueException If the IR code buffer is empty
		 * @throws BadValueException If the shader description contains invalid data
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<Shader> Create(std::vector<unsigned char>&& shaderIR, ShaderDescription desc, const std::string& addr) {
			return std::shared_ptr<Shader>(new Shader(std::move(shaderIR), desc, addr));
		}

		///@cond
		Shader(const Shader&) = delete;
		Shader(Shader&&);
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&&);
		///@endcond

		/**
		 * @brief Convert the shader data into a form suitable for rendering
		 *
		 * @throws BadRealizeStateException If the shader is already realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Realize();

		/**
		 * @brief Destroy the realized representation of the asset
		 *
		 * @throws BadRealizeStateException If the shader is not realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void DropRealized();

		///@cond
		class Impl;
		///@endcond

		~Shader();

	  private:
		Shader(std::vector<unsigned char>&& shaderIR, ShaderDescription desc, const std::string& addr);
		friend class ResourceManager;
		friend class PAL;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;
	};
}