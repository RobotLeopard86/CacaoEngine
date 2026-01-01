#pragma once

#include "impl/Shader.hpp"

#include "VulkanModule.hpp"

namespace Cacao {
	class VulkanShaderImpl : public Shader::Impl {
	  public:
		void Realize(bool& success) override;
		void DropRealized() override;

		//SPIR-V code
		std::vector<uint32_t> spv;

		//Pipeline object and layout
		vk::Pipeline pipeline;
		vk::PipelineLayout layout;

		//Descriptor set layout
		vk::DescriptorSetLayout descriptorLayout;
	};
}