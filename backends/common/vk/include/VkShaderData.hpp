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
		vk::Pipeline pipeline;						   //Graphics pipeline
		vk::DescriptorPool dpool;					   //Descriptor pool
		vk::DescriptorSet dset;						   //Descriptor set
		Allocated<vk::Buffer> localsUBO;			   //Locals uniform buffer
		void* locals;								   //Memory mapped to the locals uniform buffer
		unsigned char* shaderData;					   //Shader data buffer
		std::vector<uint32_t> vertexCode, fragmentCode;//Vertex and fragment SPIR-V code
		uint32_t shaderDataSize;					   //Total size of shader data
		std::map<std::string, uint32_t> offsets;	   //Named offsets into the shader data (how to arrange the shader data)
		std::map<std::string, int> imageSlots;		   //List of valid image slots
		bool pushConstantFromFragment;				   //If the push constant block was in the fragment shader
		vk::PipelineLayout pipelineLayout;			   //Pipeline layout
	};

	//Actual implementation of the shader native data, containing the above struct
	struct Shader::ShaderData {
		VkShaderData impl;
	};

	//Raw Vulkan texture (for shader uploading in text)
	struct RawVkTexture {
		vk::ImageView view;
		vk::Sampler sampler;
		int* slot;
	};
}