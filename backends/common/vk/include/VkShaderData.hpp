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
			int binding;					//Shader binding
			spv::Dim dimensionality;		//2D texture or cubemap?
			vk::Sampler sampler;			//Texture sampler
			vk::SamplerAddressMode wrapMode;//Texture wrapping behavior (ignored for cubemaps)
		};
		vk::Pipeline pipeline;						   //Graphics pipeline
		unsigned char* shaderData;					   //Shader data buffer
		std::vector<uint32_t> vertexCode, fragmentCode;//Vertex and fragment SPIR-V code
		uint32_t shaderDataSize;					   //Total size of shader data
		std::map<std::string, uint32_t> offsets;	   //Named offsets into the shader data (how to arrange the shader data)
		std::map<std::string, ImageSlot> imageSlots;   //List of valid image slots
		vk::PipelineLayout pipelineLayout;			   //Pipeline layout
		vk::DescriptorSetLayout setLayout;			   //Descriptor set layout
	};

	//Actual implementation of the shader native data, containing the above struct
	struct Shader::ShaderData {
		VkShaderData impl;
	};

	//Raw Vulkan texture (for shader uploading in text)
	struct RawVkTexture {
		vk::ImageView view;
		int* slot;
	};

	//I hate this
	//Controls what attributes the vertex buffer designed for a shader has
	inline enum class ShaderCompileMode {
		Standard,
		VertexOnly,
		VertexAndTexCoord
	} compileMode;

	//Controls if generated samplers should clamp textures to edge
	inline bool generatedSamplersClamp2Edge = false;
}