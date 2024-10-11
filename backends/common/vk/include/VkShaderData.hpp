#pragma once

#include "vulkan/vulkan.hpp"

#include <map>
#include <string>

#include "VkUtils.hpp"

namespace Cacao {
	//Struct for data required for a Vulkan shader
	struct Shader::ShaderData {
		vk::Pipeline pipeline;					//Graphics pipeline
		vk::DescriptorPool dpool;				//Descriptor pool
		vk::DescriptorSet dset;					//Descriptor set
		uint32_t shaderDataSize;				//Total size of shader data
		std::map<std::string, uint32_t> offsets;//Named offsets into the shader data (how to arrange the shader data)
		Allocated<vk::Buffer> localsUBO;		//Locals uniform buffer
		void* locals;							//Memory mapped to the locals uniform buffer
		void* shaderData;						//Shader data buffer
	};
}