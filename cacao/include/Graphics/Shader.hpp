#pragma once

#include <vector>
#include <filesystem>
#include <map>
#include <any>
#include <future>

#include "Core/Log.hpp"
#include "Core/Assert.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Utilities/Asset.hpp"

#include "spirv_cross.hpp"
#include "glm/glm.hpp"

namespace Cacao {
	//Shader data system

	//For convienience
	using SpvType = spirv_cross::SPIRType::BaseType;

	//Shader data item information
	struct ShaderItemInfo {
		//Base type (e.g. int, float)
		spirv_cross::TypeID type;

		//Data size (x for number of vector components, y for number of vectors in a matrix)
		//Example: a vec3 would be {3, 1}, a mat4 would be {4, 4}, and a scalar would be {1, 1}
		glm::uvec2 size;

		//Name of entry
		std::string entryName;
	};

	//Represents the layout of shader data
	using ShaderSpec = std::vector<ShaderItemInfo>;

	//Item to upload to a shader
	struct ShaderUploadItem {
		//Name of target entry in shader spec
		std::string target;

		//Actual data
		//During upload, this will be attempted to cast to the type specified in the shader item
		//If this cast fails, an error will be thrown and the upload will be aborted
		//If you are targeting an image, use that texture pointer here (a texture slot will be auto-selected and this texture will be bound)
		std::any data;
	};

	//Data to upload to a shader
	using ShaderUploadData = std::vector<ShaderUploadItem>;

	//Must be implemented per-rendering API
	//Shader class
	class Shader final : public Asset {
	  public:
		//Create a shader from raw SPIR-V code loaded separately
		Shader(std::vector<uint32_t>& vertex, std::vector<uint32_t>& fragment, ShaderSpec spec);
		//Create a shader from file paths
		Shader(std::string vertex, std::string fragment, ShaderSpec spec);

		~Shader() final {
			if(compiled && bound) Unbind();
			if(compiled) Release();
		}

		//Use this shader
		void Bind();
		//Don't use this shader
		void Unbind();
		//Compile shader to be used later
		std::shared_future<void> Compile() override;
		//Delete shader when no longer needed
		void Release() override;

		//Is shader bound?
		bool IsBound() {
			return bound;
		}

		//Read-only access to the shader spec
		const ShaderSpec& GetSpec() const {
			return specification;
		}

		//Upload data to the shader
		//WARNING: Will temporarily bind shader
		void UploadData(ShaderUploadData& data);

		//Upload Cacao Engine global shader data
		static void UploadCacaoGlobals(glm::mat4 projection, glm::mat4 view);

		//Upload Cacao Engine local shader data
		void UploadCacaoLocals(glm::mat4 transform);

		std::string GetType() override {
			return "SHADER";
		}

	  private:
		//Backend-implemented data type
		struct ShaderData;

		bool bound;
		std::shared_ptr<ShaderData> nativeData;
		const ShaderSpec specification;
	};
}