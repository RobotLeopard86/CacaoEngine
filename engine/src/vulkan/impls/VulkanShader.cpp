#include "VulkanShader.hpp"
#include "Cacao/Exceptions.hpp"
#include "VulkanModule.hpp"
#include "Targetgen.hpp"

#include "libcacaoasset.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace Cacao {
	void VulkanShaderImpl::Bake(bool& success) {
		//Convert Slang IR to SPIR-V
		libcacaoasset::Shader shdr = {};
		shdr.irCode = irBuffer;
		shdr.descriptor = descriptor;
		spv = GenerateSPV(shdr);

		//Create shader module
		vk::ShaderModuleCreateInfo moduleCI({}, spv.size() * sizeof(uint32_t), spv.data());
		vk::ShaderModule module = vulkan->dev.createShaderModule(moduleCI);

		//Create shader stage information
		std::array<vk::PipelineShaderStageCreateInfo, 2> stages;
		const auto [vertEntrypoint, fragEntrypoint] = GetEntrypointNames(shdr);
		stages[0] = vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, module, vertEntrypoint.c_str());
		stages[1] = vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, module, fragEntrypoint.c_str());

		//Input assembly info
		vk::PipelineInputAssemblyStateCreateInfo assemblyCI({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

		//Rasterization info
		vk::PipelineRasterizationStateCreateInfo rasterizerOpaqueCI(
			{}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise, VK_FALSE, 0, 0, 0, 1.0f);
		vk::PipelineRasterizationStateCreateInfo rasterizerTransparentCI(
			{}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
			vk::FrontFace::eCounterClockwise, VK_FALSE, 0, 0, 0, 1.0f);
		vk::PipelineMultisampleStateCreateInfo multisamplingCI({}, vk::SampleCountFlagBits::e1, VK_FALSE);

		//Dummy viewport state (this is dynamic state but we still need to provide a default)
		vk::PipelineViewportStateCreateInfo viewportCI({}, 1, nullptr, 1, nullptr);

		//Depth-stencil attachments
		vk::PipelineDepthStencilStateCreateInfo depthStencilOpaqueCI(
			{}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess,
			VK_FALSE, VK_FALSE, {}, {}, 1.0f, 1.0f);
		vk::PipelineDepthStencilStateCreateInfo depthStencilTransparentCI(
			{}, VK_TRUE, VK_FALSE, vk::CompareOp::eLess,
			VK_FALSE, VK_FALSE, {}, {}, 1.0f, 1.0f);
		if(customSettings) {
			if(customSettings->depth == CustomCompileSettings::Depth::Lequal) {
				depthStencilOpaqueCI.depthCompareOp = vk::CompareOp::eLessOrEqual;
				depthStencilTransparentCI.depthCompareOp = vk::CompareOp::eLessOrEqual;
			} else if(customSettings->depth == CustomCompileSettings::Depth::Off) {
				depthStencilOpaqueCI.depthCompareOp = vk::CompareOp::eAlways;
				depthStencilTransparentCI.depthCompareOp = vk::CompareOp::eAlways;
				depthStencilOpaqueCI.depthTestEnable = VK_FALSE;
				depthStencilTransparentCI.depthTestEnable = VK_FALSE;
			}
		}

		//Color attachments
		vk::PipelineColorBlendAttachmentState colorBlendAttachment(
			VK_TRUE, (customSettings && customSettings->blendUseOne) ? vk::BlendFactor::eOne : vk::BlendFactor::eSrcAlpha,
			vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd, (customSettings && customSettings->blendUseOne) ? vk::BlendFactor::eOne : vk::BlendFactor::eSrcAlpha,
			vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		vk::PipelineColorBlendStateCreateInfo colorBlendCI({}, VK_FALSE, vk::LogicOp::eCopy, colorBlendAttachment, {0.0f, 0.0f, 0.0f, 0.0f});

		//Dynamic state information
		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynStateCI({}, dynamicStates);

		//Pipeline rendering info
		vk::PipelineRenderingCreateInfo pipelineRenderingInfo(0, vulkan->surfaceFormat.format, vulkan->selectedDF);

		//Create input attributes and bindings
		vk::VertexInputBindingDescription inputBinding(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
		std::array<vk::VertexInputAttributeDescription, 4> inputAttrs {{{0, 0, vk::Format::eR32G32B32Sfloat, 0},
			{1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoords)},
			{2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
			{3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent)}}};
		vk::PipelineVertexInputStateCreateInfo inputStateCI({}, inputBinding, inputAttrs);
		if(descriptor.domain == libcacaoasset::Shader::Descriptor::Domain::Canvas2D) {
			inputBinding.stride = sizeof(float) * 5;
			inputStateCI.setVertexBindingDescriptions(inputBinding);
			std::array<vk::VertexInputAttributeDescription, 2> vtcAttrs = {inputAttrs[0], inputAttrs[1]};
			inputStateCI.setVertexAttributeDescriptions(vtcAttrs);
		}

		//Create pipeline layout
		vk::PushConstantRange pcr {vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, (sizeof(glm::mat4) + sizeof(glm::mat3) + sizeof(uint32_t) + sizeof(float) * 4)};
		vk::PipelineLayoutCreateInfo layoutCI;
		if(descriptor.uniformParams.size() > 0 || descriptor.texParams.size() > 0) {
			std::vector<vk::DescriptorSetLayoutBinding> matDSBindings;
			if(descriptor.uniformParams.size() > 0) matDSBindings.emplace_back(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, VK_NULL_HANDLE);
			for(auto texParam : descriptor.texParams) {
				matDSBindings.emplace_back(texParam.binding, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, VK_NULL_HANDLE);
			}
			matLayout = vulkan->dev.createDescriptorSetLayout({vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR, matDSBindings});
			std::array<vk::DescriptorSetLayout, 2> layouts {{vulkan->engineSetLayout, matLayout}};
			layoutCI = vk::PipelineLayoutCreateInfo({}, layouts, pcr);
		} else {
			layoutCI = vk::PipelineLayoutCreateInfo({}, vulkan->engineSetLayout, pcr);
		}
		layout = vulkan->dev.createPipelineLayout(layoutCI);

		//Create opaque pipeline
		vk::GraphicsPipelineCreateInfo pipelineOpaqueCI({}, stages, &inputStateCI, &assemblyCI, nullptr, &viewportCI, &rasterizerOpaqueCI,
			&multisamplingCI, &depthStencilOpaqueCI, &colorBlendCI, &dynStateCI, layout);
		pipelineOpaqueCI.pNext = &pipelineRenderingInfo;
		auto pipelineOpaqueResult = vulkan->dev.createGraphicsPipeline({}, pipelineOpaqueCI);
		Check<ExternalException>(pipelineOpaqueResult.result == vk::Result::eSuccess, "Failed to create shader opaque rendering pipeline!");
		pipelineOpaque = pipelineOpaqueResult.value;

		//Create transparent pipeline
		vk::GraphicsPipelineCreateInfo pipelineTransparentCI({}, stages, &inputStateCI, &assemblyCI, nullptr, &viewportCI, &rasterizerTransparentCI,
			&multisamplingCI, &depthStencilTransparentCI, &colorBlendCI, &dynStateCI, layout);
		pipelineTransparentCI.pNext = &pipelineRenderingInfo;
		auto pipelineTransparentResult = vulkan->dev.createGraphicsPipeline({}, pipelineTransparentCI);
		Check<ExternalException>(pipelineTransparentResult.result == vk::Result::eSuccess, "Failed to create shader transparent rendering pipeline!");
		pipelineTransparent = pipelineTransparentResult.value;

		//Free shader modules
		vulkan->dev.destroyShaderModule(module);

		//Create material data UBO
		if(descriptor.uniformParams.size() > 0) {
			//Create buffer
			vk::BufferCreateInfo uboCI({}, sizeof(glm::mat4) * 3 + sizeof(glm::vec3), vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
			vma::AllocationCreateInfo uboAllocCI({}, vma::MemoryUsage::eCpuToGpu);
			try {
				materialData = vulkan->allocator.createBuffer(uboCI, uboAllocCI);
			} catch(vk::SystemError& err) {
				std::stringstream emsg;
				emsg << "Could not create shader's material data uniform buffer: " << err.what();
				Check<ExternalException>(false, emsg.str());
			}

			//Map buffer memory
			Check<ExternalException>(vulkan->allocator.mapMemory(materialData.alloc, &materialData.mem) == vk::Result::eSuccess, "Could not map camera data uniform buffer memory!");
		}

		success = true;
	}

	void VulkanShaderImpl::Discard() {
		if(descriptor.uniformParams.size() > 0) {
			vulkan->allocator.unmapMemory(materialData.alloc);
			vulkan->allocator.destroyBuffer(materialData.obj, materialData.alloc);
		}
		vulkan->dev.destroyPipeline(pipelineOpaque);
		vulkan->dev.destroyPipeline(pipelineTransparent);
		vulkan->dev.destroyPipelineLayout(layout);
		if(descriptor.uniformParams.size() > 0 || descriptor.texParams.size() > 0) vulkan->dev.destroyDescriptorSetLayout(matLayout);
	}

	void VulkanShaderImpl::Bind(VulkanCommandBuffer* vcb, bool transparent) {
		//Bind pipeline object
		vcb->cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, transparent ? pipelineTransparent : pipelineOpaque);

		//Bind engine descriptor set
		vcb->BindSet(layout);

		//Push material data UBO descriptor
		if(descriptor.uniformParams.size() > 0) {
			vk::DescriptorBufferInfo matDBI(materialData.obj, 0, vk::WholeSize);
			vk::WriteDescriptorSet set1(VK_NULL_HANDLE, 0, 0, 1, vk::DescriptorType::eUniformBuffer, VK_NULL_HANDLE, &matDBI);
			vcb->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, layout, 1, set1);
		}
	}

	Shader::Impl* VulkanModule::ConfigureShader() {
		return new VulkanShaderImpl();
	}
}