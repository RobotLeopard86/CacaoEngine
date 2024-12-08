#pragma once

#include "vulkan/vulkan.hpp"

#include <map>
#include <string>

#include "VkUtils.hpp"
#include "Graphics/Shader.hpp"

namespace Cacao {
	//Struct for data required for a Vulkan shader
	//This is separate because it needs to be visible to the rest of the backend
	struct VkShaderData {
		struct ImageSlot {
			int binding;			//Shader binding
			spv::Dim dimensionality;//2D texture or cubemap?
			vk::Sampler sampler;	//Texture sampler
		};
		vk::Pipeline pipeline;						   //Graphics pipeline
		std::vector<uint32_t> vertexCode, fragmentCode;//Vertex and fragment SPIR-V code
		uint32_t shaderDataSize;					   //Total size of shader data
		std::map<std::string, uint32_t> offsets;	   //Named offsets into the shader data (how to arrange the shader data)
		std::map<std::string, ImageSlot> imageSlots;   //List of valid image slots
		vk::PipelineLayout pipelineLayout;			   //Pipeline layout
		vk::DescriptorSetLayout setLayout;			   //Descriptor set layout
		bool usesCustomCompile;						   //If custom compilation (manual invoking of the pipeline generator is allowed). If so, then Shader::Compile does no actual compilation but marks itself as compiled.
	};

	//Shader compilation settings
	struct ShaderCompileSettings {
		std::map<std::string, vk::SamplerAddressMode> wrapModes;
		enum class InputType {
			Standard,
			VertexOnly,
			VertexAndTexCoord
		} input;
		enum class Blending {
			Standard,
			One,
			Src
		} blend;
		enum class Depth {
			Off,
			Less,
			Lequal
		} depth;
	};

	//Does the actual compilation work on a VkShaderData
	//Shader::Compile calls this, but it's useful for doing non-standard compilation
	void DoVkShaderCompile(VkShaderData* shader, const ShaderCompileSettings& settings);

	//Actual implementation of the shader native data, containing the above struct
	struct Shader::ShaderData {
		VkShaderData impl;
	};

	//Mapping of shader pointers to their shader data (allows for custom compilation)
	inline std::map<Shader*, VkShaderData*> shaderDataLookup;

	//Raw Vulkan texture (for shader uploading in text)
	struct RawVkTexture {
		vk::ImageView view;
		int* slot;
	};
}