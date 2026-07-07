#pragma once

#include "impl/Shader.hpp"

#include "VulkanModule.hpp"

namespace Cacao {
	class VulkanShaderImpl : public Shader::Impl {
	  public:
		void Bake(bool& success) override;
		void Discard() override;

		//SPIR-V code
		std::vector<uint32_t> spv;

		//Pipeline object and layout
		vk::Pipeline pipelineOpaque;
		vk::Pipeline pipelineTransparent;
		vk::PipelineLayout layout;

		//Descriptor set layouts
		vk::DescriptorSetLayout cacaoLayout;
		vk::DescriptorSetLayout matLayout;//optional

		//Material data UBO (optional)
		MappedBuffer materialData;

		//Custom shader compilation settings
		struct CustomCompileSettings {
			bool blendUseSrc;
			enum class Depth {
				Off,
				Less,
				Lequal
			} depth;
		};
		std::optional<CustomCompileSettings> customSettings;
	};
}