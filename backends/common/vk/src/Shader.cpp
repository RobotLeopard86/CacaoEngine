#include "Graphics/Shader.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "VkShader.hpp"
#include "VulkanCoreObjects.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"
#include "3D/Vertex.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "Graphics/Textures/Texture2D.hpp"
#include "Utilities/AssetManager.hpp"

#include <future>
#include <filesystem>

#include "spirv_reflect.hpp"
#include "spirv_cross.hpp"
#include "spirv_parser.hpp"

//See VkShader.hpp for why there's this "impl" thing
//Basically this just makes it easier to use and keeps the arrow operator method of
//accessing native data intact
#define nd (&(this->nativeData->impl))

namespace Cacao {
	//Sets up shader data according to the shader spec
	void PrepShaderData(const ShaderSpec& spec, VkShaderData* mod) {
		CheckException(mod, Exception::GetExceptionCodeFromMeaning("NullValue"), "Passed-in shader data pointer is invalid!");

		//Copy the SPIR-V because SPIRV-Cross requires a move
		std::vector<uint32_t> vc = mod->vertexCode;
		std::vector<uint32_t> fc = mod->fragmentCode;

		//Parse SPIR-V IR
		spirv_cross::Parser vertParse(std::move(vc));
		vertParse.parse();
		spirv_cross::Parser fragParse(std::move(fc));
		fragParse.parse();

		//Create reflection compilers
		spirv_cross::CompilerReflection vertReflector(vertParse.get_parsed_ir());
		spirv_cross::ShaderResources vr = vertReflector.get_shader_resources();
		spirv_cross::CompilerReflection fragReflector(fragParse.get_parsed_ir());
		spirv_cross::ShaderResources fr = fragReflector.get_shader_resources();

		//Get shader data block size and offsets
		bool foundSD = false;
		for(auto ubo : vr.uniform_buffers) {
			if(ubo.name.compare("ObjectData") == 0 || ubo.name.compare("object_data") == 0 || ubo.name.compare("type.ConstantBuffer.ObjectData") == 0 || ubo.name.compare("object") == 0) {
				spirv_cross::SPIRType type = vertReflector.get_type(ubo.base_type_id);
				mod->shaderDataSize = vertReflector.get_declared_struct_size(type);
				for(unsigned int i = 0; i < type.member_types.size(); ++i) {
					mod->offsets.insert_or_assign(vertReflector.get_member_name(ubo.base_type_id, i), vertReflector.type_struct_member_offset(type, i));
				}
				foundSD = true;
				break;
			}
		}
		if(!foundSD) {
			mod->shaderDataSize = 0;
		}

		//Get valid image slots
		for(auto tex : fr.sampled_images) {
			VkShaderData::ImageSlot slotVal = {};
			slotVal.binding = fragReflector.get_decoration(tex.id, spv::DecorationBinding);
			slotVal.dimensionality = fragReflector.get_type(tex.base_type_id).image.dim;
			mod->imageSlots.insert_or_assign(fragReflector.get_name(tex.id), slotVal);
		}

		//Make sure that shader data matches spec
		for(const ShaderItemInfo& sii : spec) {
			CheckException((sii.type == SpvType::SampledImage && mod->imageSlots.contains(sii.name)) || mod->offsets.contains(sii.name), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Value found in shader spec that is not present in shader!");
		}
		for(auto offset : mod->offsets) {
			CheckException(std::find_if(spec.begin(), spec.end(), [&offset](const ShaderItemInfo& sii) { return offset.first.compare(sii.name) == 0; }) != spec.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Value found in shader that is not in shader spec!");
		}
		for(auto slot : mod->imageSlots) {
			CheckException(std::find_if(spec.begin(), spec.end(), [&slot](const ShaderItemInfo& sii) { return slot.first.compare(sii.name) == 0 && sii.type == SpvType::SampledImage; }) != spec.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Texture found in shader that is not in shader spec!");
		}
	}

	Shader::Shader(std::string vertexPath, std::string fragmentPath, ShaderSpec spec)
	  : Asset(false), bound(false), specification(spec) {
		//Validate that these paths exist
		CheckException(std::filesystem::exists(vertexPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create a shader from a non-existent vertex shader file!");
		CheckException(std::filesystem::exists(fragmentPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create a shader from a non-existent fragment shader file!");

		//Load SPIR-V code

		//Open file streams
		FILE* vf = fopen(vertexPath.c_str(), "rb");
		CheckException(vf, Exception::GetExceptionCodeFromMeaning("FileOpenFailure"), "Failed to open vertex shader file!");
		FILE* ff = fopen(fragmentPath.c_str(), "rb");
		CheckException(ff, Exception::GetExceptionCodeFromMeaning("FileOpenFailure"), "Failed to open fragment shader file!");

		//Get size of shader files
		fseek(vf, 0, SEEK_END);
		long vlen = ftell(vf) / sizeof(uint32_t);
		rewind(vf);
		fseek(ff, 0, SEEK_END);
		long flen = ftell(ff) / sizeof(uint32_t);
		rewind(ff);

		//Load shader data into vectors
		std::vector<uint32_t> vbuf(vlen);
		std::vector<uint32_t> fbuf(flen);
		if(fread(vbuf.data(), sizeof(uint32_t), vlen, vf) != std::size_t(vlen)) {
			//Clean up file streams before exception throw
			fclose(vf);
			fclose(ff);
			CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to read vertex shader data from file!");
		}
		if(fread(fbuf.data(), sizeof(uint32_t), flen, ff) != std::size_t(flen)) {
			//Clean up file streams before exception throw
			fclose(vf);
			fclose(ff);
			CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to read fragment shader data from file!");
		}

		//Close file streams
		fclose(vf);
		fclose(ff);

		//Create native data
		nativeData.reset(new ShaderData());
		nd->vertexCode = vbuf;
		nd->fragmentCode = fbuf;

		//Register native data
		shaderDataLookup.insert_or_assign(this, &(nativeData->impl));

		//Prepare native data
		PrepShaderData(spec, nd);
	}

	Shader::Shader(std::vector<uint32_t>& vertex, std::vector<uint32_t>& fragment, ShaderSpec spec)
	  : Asset(false), bound(false), specification(spec) {
		//Create native data
		nativeData.reset(new ShaderData());

		//Create native data
		nativeData.reset(new ShaderData());
		nd->vertexCode = vertex;
		nd->fragmentCode = fragment;

		//Register native data
		shaderDataLookup.insert_or_assign(this, &(nativeData->impl));

		//Prepare native data
		PrepShaderData(spec, nd);
	}

	std::shared_future<void> Shader::CompileAsync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled shader!");
		return Engine::GetInstance()->GetThreadPool()->enqueue([this]() { this->CompileSync(); }).share();
	}

	void Shader::CompileSync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled shader!");

		//If custom compilation is not used, invoke the compiler with default settings
		if(!nd->usesCustomCompile) {
			ShaderCompileSettings settings {};
			settings.blend = ShaderCompileSettings::Blending::Standard;
			settings.depth = ShaderCompileSettings::Depth::Less;
			settings.input = ShaderCompileSettings::InputType::Standard;
			for(auto kv : nd->imageSlots) {
				settings.wrapModes.insert_or_assign(kv.first, vk::SamplerAddressMode::eRepeat);
			}
			DoVkShaderCompile(nd, settings);
		}

		compiled = true;
	}

	void Shader::_BackendDestruct() {
		if(auto it = shaderDataLookup.find(this); it != shaderDataLookup.end()) shaderDataLookup.erase(it);
	}

	void DoVkShaderCompile(VkShaderData* shader, const ShaderCompileSettings& settings) {
		//Create shader modules
		vk::ShaderModuleCreateInfo vertexModCI({}, shader->vertexCode.size() * sizeof(uint32_t), reinterpret_cast<const uint32_t*>(shader->vertexCode.data()));
		vk::ShaderModule vertexMod = dev.createShaderModule(vertexModCI);
		vk::ShaderModuleCreateInfo fragmentModCI({}, shader->fragmentCode.size() * sizeof(uint32_t), reinterpret_cast<const uint32_t*>(shader->fragmentCode.data()));
		vk::ShaderModule fragmentMod = dev.createShaderModule(fragmentModCI);
		vk::PipelineShaderStageCreateInfo vertexStageCI({}, vk::ShaderStageFlagBits::eVertex, vertexMod, "main");
		vk::PipelineShaderStageCreateInfo fragmentStageCI({}, vk::ShaderStageFlagBits::eFragment, fragmentMod, "main");
		std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vertexStageCI, fragmentStageCI};

		//Create input assembly info
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCI({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

		//Create rasterization info
		vk::PipelineRasterizationStateCreateInfo rasterizerCI(
			{}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise, VK_FALSE, 0, 0, 0, 1.0f);
		vk::PipelineMultisampleStateCreateInfo multisamplingCI({}, vk::SampleCountFlagBits::e1, VK_FALSE);

		//Define viewport state
		vk::PipelineViewportStateCreateInfo viewportState({}, 1, nullptr, 1, nullptr);

		//Create color and depth-stencil attachments
		vk::PipelineDepthStencilStateCreateInfo depthStencilCI(
			{}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess,
			VK_FALSE, VK_FALSE, {}, {}, 1.0f, 1.0f);
		if(settings.depth == ShaderCompileSettings::Depth::Lequal) {
			depthStencilCI.depthCompareOp = vk::CompareOp::eLessOrEqual;
		} else if(settings.depth == ShaderCompileSettings::Depth::Off) {
			depthStencilCI.depthCompareOp = vk::CompareOp::eAlways;
			depthStencilCI.depthTestEnable = VK_FALSE;
		}
		vk::PipelineColorBlendAttachmentState colorBlendAttach(
			VK_FALSE, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		if(settings.blend != ShaderCompileSettings::Blending::Standard) {
			colorBlendAttach.blendEnable = VK_TRUE;
			colorBlendAttach.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
			colorBlendAttach.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
			if(settings.blend == ShaderCompileSettings::Blending::Src) {
				colorBlendAttach.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
				colorBlendAttach.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
			}
		}
		vk::PipelineColorBlendStateCreateInfo colorBlendCI({}, VK_FALSE, vk::LogicOp::eCopy, colorBlendAttach, {0.0f, 0.0f, 0.0f, 0.0f});

		//Create dynamic state info
		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynStateCI({}, dynamicStates);

		//Create pipeline rendering info
		vk::PipelineRenderingCreateInfo pipelineRenderingInfo(0, surfaceFormat.format, selectedDF);

		//Create input attributes and bindings
		vk::VertexInputBindingDescription inputBinding(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
		std::array<vk::VertexInputAttributeDescription, 5> inputAttrs {{{0, 0, vk::Format::eR32G32B32Sfloat, 0},
			{1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoords)},
			{2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, tangent)},
			{3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, bitangent)},
			{4, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)}}};
		vk::PipelineVertexInputStateCreateInfo inputStateCI({}, inputBinding, inputAttrs);
		if(settings.input == ShaderCompileSettings::InputType::VertexOnly) {
			inputBinding.stride = sizeof(float) * 3;
			inputStateCI.setVertexBindingDescriptions(inputBinding);
			inputStateCI.setVertexAttributeDescriptions(inputAttrs[0]);
		} else if(settings.input == ShaderCompileSettings::InputType::VertexAndTexCoord) {
			inputBinding.stride = sizeof(float) * 5;
			inputStateCI.setVertexBindingDescriptions(inputBinding);
			std::array<vk::VertexInputAttributeDescription, 2> vtcAttrs = {inputAttrs[0], inputAttrs[1]};
			inputStateCI.setVertexAttributeDescriptions(vtcAttrs);
		}

		//Create image slot samplers
		for(auto slot : shader->imageSlots) {
			if(slot.second.dimensionality == spv::Dim::DimCube) {
				vk::SamplerCreateInfo samplerCI({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge,
					vk::SamplerAddressMode::eClampToEdge, 0.0f, VK_FALSE, 0.0f, VK_FALSE, vk::CompareOp::eNever, 0.0f, 0.0f, vk::BorderColor::eIntTransparentBlack, VK_FALSE);
				shader->imageSlots[slot.first].sampler = dev.createSampler(samplerCI);
			} else {
				vk::SamplerCreateInfo samplerCI({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, settings.wrapModes.at(slot.first), settings.wrapModes.at(slot.first),
					settings.wrapModes.at(slot.first), 0.0f, VK_FALSE, 0.0f, VK_FALSE, vk::CompareOp::eNever, 0.0f, VK_REMAINING_MIP_LEVELS, vk::BorderColor::eIntTransparentBlack, VK_FALSE);
				shader->imageSlots[slot.first].sampler = dev.createSampler(samplerCI);
			}
		}

		//Create descriptor set layout
		std::vector<vk::DescriptorSetLayoutBinding> dsBindings;
		dsBindings.emplace_back(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, VK_NULL_HANDLE);
		if(shader->shaderDataSize > 0) dsBindings.emplace_back(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, VK_NULL_HANDLE);
		for(auto slot : shader->imageSlots) {
			dsBindings.emplace_back(slot.second.binding, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, &slot.second.sampler);
		}
		shader->setLayout = dev.createDescriptorSetLayout({vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR, dsBindings});

		//Create pipeline layout
		vk::PushConstantRange pcr(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4));
		vk::PipelineLayoutCreateInfo layoutCI({}, shader->setLayout, pcr);
		shader->pipelineLayout = dev.createPipelineLayout(layoutCI);

		//Create pipeline
		vk::GraphicsPipelineCreateInfo pipelineCI({}, shaderStages, &inputStateCI, &inputAssemblyCI, nullptr, &viewportState, &rasterizerCI,
			&multisamplingCI, &depthStencilCI, &colorBlendCI, &dynStateCI, shader->pipelineLayout);
		pipelineCI.pNext = &pipelineRenderingInfo;
		auto pipelineResult = dev.createGraphicsPipeline({}, pipelineCI);
		CheckException(pipelineResult.result == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("Vulkan"), "Failed to create shader pipeline!");
		shader->pipeline = pipelineResult.value;

		//Free shader modules now that we don't need them
		dev.destroyShaderModule(vertexMod);
		dev.destroyShaderModule(fragmentMod);
	}

	void Shader::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled shader!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release bound shader!");

		//Destroy image slot samplers
		for(auto slotPair : nd->imageSlots) {
			dev.destroySampler(slotPair.second.sampler);
		}

		//Destroy pipeline
		dev.destroyPipeline(nd->pipeline);

		//Destroy pipeline layout
		dev.destroyPipelineLayout(nd->pipelineLayout);

		//Destroy descriptor set layout
		dev.destroyDescriptorSetLayout(nd->setLayout);

		compiled = false;
	}

	void Shader::Bind() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled shader!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound shader!");
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot bind shader when there is no active frame object!");

		//Bind pipeline object
		activeFrame->cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, nd->pipeline);

		//Bind globals UBO
		vk::DescriptorBufferInfo globalsDBI(globalsUBO.obj, 0, vk::WholeSize);
		vk::WriteDescriptorSet dsWrite(VK_NULL_HANDLE, 0, 0, 1, vk::DescriptorType::eUniformBuffer, VK_NULL_HANDLE, &globalsDBI);
		activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, nd->pipelineLayout, 0, dsWrite);

		//Mark us as the active shader
		activeShader = nd;

		bound = true;
	}

	void Shader::Unbind() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot unbind uncompiled shader!");
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound shader!");

		//Make us not the active shader
		if(activeShader == nd) activeShader = nullptr;
		std::stringstream pp;

		//Vulkan has no concept of unbinding a pipeline, also since command buffers shift around state is wiped clean every time
		bound = false;
	}

	void Shader::UploadCacaoGlobals(glm::mat4 projection, glm::mat4 view) {
		glm::mat4 cvp[2] = {projection, view};
		std::memcpy(globalsMem, cvp, sizeof(glm::mat4) * 2);
	}

	std::shared_ptr<Material> Shader::CreateMaterial() {
		AssetHandle<Shader> selfHandle = AssetManager::GetInstance()->GetHandleFromPointer(this);

		//Unfortunately, we have to do it this way because make_shared doesn't work well with friend classes
		Material* m;
		if(selfHandle.IsNull()) {
			//We should never have to do this except for engine-internal shaders that aren't cached
			m = new Material(this);
		} else {
			m = new Material(selfHandle);
		}
		return std::shared_ptr<Material>(m);
	}
}