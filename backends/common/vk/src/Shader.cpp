#include "Graphics/Shader.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "VkShaderData.hpp"
#include "VulkanCoreObjects.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"
#include "3D/Vertex.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "Graphics/Textures/Texture2D.hpp"

#include <future>
#include <filesystem>

#include "spirv_reflect.hpp"
#include "spirv_cross.hpp"
#include "spirv_parser.hpp"
#include "glm/gtc/type_ptr.hpp"

//See VkShaderData.hpp for why there's this "impl" thing
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
		for(auto pcb : vr.push_constant_buffers) {
			if(pcb.name.compare("type.PushConstant.ObjectData") == 0 || pcb.name.compare("object") == 0) {
				spirv_cross::SPIRType type = vertReflector.get_type(pcb.base_type_id);
				mod->shaderDataSize = vertReflector.get_declared_struct_size(type);
				for(unsigned int i = 0; i < type.member_types.size(); ++i) {
					mod->offsets.insert_or_assign(vertReflector.get_member_name(pcb.base_type_id, i), vertReflector.type_struct_member_offset(type, i));
				}
				foundSD = true;
				break;
			}
		}
		CheckException(foundSD, Exception::GetExceptionCodeFromMeaning("Vulkan"), "Shaders must contain the ObjectData push constant block!");

		//Get valid image slots
		for(auto tex : fr.sampled_images) {
			VkShaderData::ImageSlot slotVal = {};
			slotVal.binding = fragReflector.get_decoration(tex.id, spv::DecorationBinding);
			slotVal.dimensionality = fragReflector.get_type(tex.base_type_id).image.dim;
			slotVal.wrapMode = generatedSamplersClamp2Edge ? vk::SamplerAddressMode::eClampToEdge : vk::SamplerAddressMode::eRepeat;
			mod->imageSlots.insert_or_assign(fragReflector.get_name(tex.id), slotVal);
		}

		//Make sure that shader data matches spec
		for(const ShaderItemInfo& sii : spec) {
			CheckException((sii.type == SpvType::SampledImage && mod->imageSlots.contains(sii.entryName)) || mod->offsets.contains(sii.entryName), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Value found in shader spec that is not present in shader!");
		}
		for(auto offset : mod->offsets) {
			if(offset.first.compare("transform") == 0) continue;
			CheckException(std::find_if(spec.begin(), spec.end(), [&offset](const ShaderItemInfo& sii) { return offset.first.compare(sii.entryName) == 0; }) != spec.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Value found in shader that is not in shader spec!");
		}
		for(auto slot : mod->imageSlots) {
			CheckException(std::find_if(spec.begin(), spec.end(), [&slot](const ShaderItemInfo& sii) { return slot.first.compare(sii.entryName) == 0 && sii.type == SpvType::SampledImage; }) != spec.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Texture found in shader that is not in shader spec!");
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

		//Prepare native data
		PrepShaderData(spec, nd);
	}

	std::shared_future<void> Shader::CompileAsync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled shader!");
		return Engine::GetInstance()->GetThreadPool()->enqueue([this]() { this->CompileSync(); }).share();
	}

	void Shader::CompileSync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled shader!");
		//Create shader modules
		vk::ShaderModuleCreateInfo vertexModCI({}, nd->vertexCode.size() * sizeof(uint32_t), reinterpret_cast<const uint32_t*>(nd->vertexCode.data()));
		vk::ShaderModule vertexMod = dev.createShaderModule(vertexModCI);
		vk::ShaderModuleCreateInfo fragmentModCI({}, nd->fragmentCode.size() * sizeof(uint32_t), reinterpret_cast<const uint32_t*>(nd->fragmentCode.data()));
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
		vk::PipelineColorBlendAttachmentState colorBlendAttach(
			VK_FALSE, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		vk::PipelineColorBlendStateCreateInfo colorBlendCI({}, VK_FALSE, vk::LogicOp::eCopy, colorBlendAttach, {0.0f, 0.0f, 0.0f, 0.0f});

		//Create dynamic state info
		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
			vk::DynamicState::eColorBlendEquationEXT,
			vk::DynamicState::eColorBlendEnableEXT,
			vk::DynamicState::eDepthTestEnable,
			vk::DynamicState::eDepthCompareOp};
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
		if(compileMode == ShaderCompileMode::VertexOnly) {
			inputBinding.stride = sizeof(float) * 3;
			inputStateCI.setVertexBindingDescriptions(inputBinding);
			inputStateCI.setVertexAttributeDescriptions(inputAttrs[0]);
		} else if(compileMode == ShaderCompileMode::VertexAndTexCoord) {
			inputBinding.stride = sizeof(float) * 5;
			inputStateCI.setVertexBindingDescriptions(inputBinding);
			std::array<vk::VertexInputAttributeDescription, 2> vtcAttrs = {inputAttrs[0], inputAttrs[1]};
			inputStateCI.setVertexAttributeDescriptions(vtcAttrs);
		}

		//Create image slot samplers
		for(auto slot : nd->imageSlots) {
			if(slot.second.dimensionality == spv::Dim::DimCube) {
				vk::SamplerCreateInfo samplerCI({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge,
					vk::SamplerAddressMode::eClampToEdge, 0.0f, VK_FALSE, 0.0f, VK_FALSE, vk::CompareOp::eNever, 0.0f, 0.0f, vk::BorderColor::eIntTransparentBlack, VK_FALSE);
				nd->imageSlots[slot.first].sampler = dev.createSampler(samplerCI);
			} else {
				vk::SamplerCreateInfo samplerCI({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, slot.second.wrapMode, slot.second.wrapMode,
					slot.second.wrapMode, 0.0f, VK_FALSE, 0.0f, VK_FALSE, vk::CompareOp::eNever, 0.0f, VK_REMAINING_MIP_LEVELS, vk::BorderColor::eIntTransparentBlack, VK_FALSE);
				nd->imageSlots[slot.first].sampler = dev.createSampler(samplerCI);
			}
		}

		//Create descriptor set layout
		std::vector<vk::DescriptorSetLayoutBinding> dsBindings;
		dsBindings.emplace_back(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, VK_NULL_HANDLE);
		for(auto slot : nd->imageSlots) {
			dsBindings.emplace_back(slot.second.binding, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, &slot.second.sampler);
		}
		nd->setLayout = dev.createDescriptorSetLayout({vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR, dsBindings});

		//Create pipeline layout
		vk::PushConstantRange pcr(vk::ShaderStageFlagBits::eVertex, 0, nd->shaderDataSize + sizeof(glm::mat4));
		vk::PipelineLayoutCreateInfo layoutCI({}, nd->setLayout, pcr);
		nd->pipelineLayout = dev.createPipelineLayout(layoutCI);

		//Create pipeline
		vk::GraphicsPipelineCreateInfo pipelineCI({}, shaderStages, &inputStateCI, &inputAssemblyCI, nullptr, &viewportState, &rasterizerCI,
			&multisamplingCI, &depthStencilCI, &colorBlendCI, &dynStateCI, nd->pipelineLayout);
		pipelineCI.pNext = &pipelineRenderingInfo;
		auto pipelineResult = dev.createGraphicsPipeline({}, pipelineCI);
		CheckException(pipelineResult.result == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("Vulkan"), "Failed to create shader pipeline!");
		nd->pipeline = pipelineResult.value;

		//Free shader modules now that we don't need them
		dev.destroyShaderModule(vertexMod);
		dev.destroyShaderModule(fragmentMod);

		//Allocate shader data memory (scary)
		nd->shaderData = (unsigned char*)malloc(nd->shaderDataSize + sizeof(glm::mat4));

		compiled = true;
	}

	void Shader::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled shader!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release bound shader!");

		//Destroy image slot samplers
		for(auto slotPair : nd->imageSlots) {
			dev.destroySampler(slotPair.second.sampler);
		}

		//Free shader data memory
		free(nd->shaderData);

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

	void Shader::UploadData(ShaderUploadData& data, const glm::mat4& transformation) {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot upload data to uncompiled shader!");
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot upload data to bound shader!");
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot upload data to shader when there is no active frame object!");
		CheckException(!(nd->shaderDataSize == 0 && nd->imageSlots.size() == 0 && data.size() > 0), Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot upload data to a shader that doesn't support data uploads!");

		//Do data upload
		std::map<std::string, ShaderItemInfo> foundItems;
		for(ShaderUploadItem& item : data) {
			//Attempt to locate item in shader spec
			bool found = false;
			if(!foundItems.contains(item.target)) {
				for(ShaderItemInfo sii : specification) {
					foundItems.insert_or_assign(sii.entryName, sii);
					if(sii.entryName.compare(item.target) == 0) {
						found = true;
						break;
					}
				}
			}
			CheckException(found, Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Can't locate item targeted by upload in shader specification!");

			//Grab shader item info
			ShaderItemInfo info = foundItems[item.target];

			//Find offset or image binding
			unsigned int offset = 0;
			VkShaderData::ImageSlot slot;
			if(info.type == SpvType::SampledImage) {
				CheckException(nd->imageSlots.contains(item.target), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Can't locate item targeted by upload in shader offsets!");
				slot = nd->imageSlots[item.target];
			} else {
				CheckException(nd->offsets.contains(item.target), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Can't locate item targeted by upload in shader offsets!");
				offset = nd->offsets[item.target];
			}

			//Turn dimensions into single number (easier for uploading)
			int dims = (4 * info.size.y) - (4 - info.size.x);
			CheckException(!(info.size.x == 1 && info.size.y >= 2), Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Shaders cannot have data with one column and 2+ rows!");
			CheckException(!(info.size.x > 1 && info.size.y > 1 && info.type != SpvType::Float && info.type != SpvType::Double), Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Shaders cannot have data with 2+ columns and rows that are not floats or doubles!");

			//Cast data to correct type to get it out of std::any
			unsigned char* data;
			std::size_t dataSize = 0;
			try {
				switch(info.type) {
					case SpvType::Boolean:
						switch(dims) {
							case 1:
								data = reinterpret_cast<unsigned char*>(std::any_cast<bool>(&item.data));
								dataSize = sizeof(bool);
								break;
							case 2:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::bvec2>(item.data))));
								dataSize = sizeof(glm::bvec2);
								break;
							case 3:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::bvec3>(item.data))));
								dataSize = sizeof(glm::bvec3);
								break;
							case 4:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::bvec4>(item.data))));
								dataSize = sizeof(glm::bvec4);
								break;
						}
						break;
					case SpvType::Int:
						switch(dims) {
							case 1:
								data = reinterpret_cast<unsigned char*>(std::any_cast<int>(&item.data));
								dataSize = sizeof(int);
								break;
							case 2:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::ivec2>(item.data))));
								dataSize = sizeof(glm::ivec2);
								break;
							case 3:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::ivec3>(item.data))));
								dataSize = sizeof(glm::ivec3);
								break;
							case 4:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::ivec4>(item.data))));
								dataSize = sizeof(glm::ivec4);
								break;
						}
						break;
					case SpvType::SampledImage:
						CheckException(dims == 1, Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Shaders cannot have arrays or matrices of textures!");

						//Bind texture to the slot specified
						if(item.data.type() == typeid(Texture2D*)) {
							Texture2D* tex = std::any_cast<Texture2D*>(item.data);
							tex->Bind(slot.binding);
						} else if(item.data.type() == typeid(Cubemap*)) {
							Cubemap* tex = std::any_cast<Cubemap*>(item.data);
							tex->Bind(slot.binding);
						} else if(item.data.type() == typeid(UIView*)) {
							UIView* view = std::any_cast<UIView*>(item.data);
							view->Bind(slot.binding);
						} else if(item.data.type() == typeid(AssetHandle<Texture2D>)) {
							AssetHandle<Texture2D> tex = std::any_cast<AssetHandle<Texture2D>>(item.data);
							tex->Bind(slot.binding);
						} else if(item.data.type() == typeid(AssetHandle<Cubemap>)) {
							AssetHandle<Cubemap> tex = std::any_cast<AssetHandle<Cubemap>>(item.data);
							tex->Bind(slot.binding);
						} else if(item.data.type() == typeid(AssetHandle<UIView>)) {
							AssetHandle<UIView> view = std::any_cast<AssetHandle<UIView>>(item.data);
							view->Bind(slot.binding);
						} else if(item.data.type() == typeid(RawVkTexture)) {
							RawVkTexture raw = std::any_cast<RawVkTexture>(item.data);
							vk::DescriptorImageInfo dii(slot.sampler, raw.view, vk::ImageLayout::eShaderReadOnlyOptimal);
							vk::WriteDescriptorSet wds(VK_NULL_HANDLE, slot.binding, 0, vk::DescriptorType::eCombinedImageSampler, dii);
							activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, nd->pipelineLayout, 0, wds);
							*(raw.slot) = slot.binding;
						} else {
							Logging::EngineLog(item.data.type().name());
							CheckException(false, Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Non-texture value supplied to texture uniform!");
						}

						break;
					case SpvType::UInt:
						switch(dims) {
							case 1:
								data = reinterpret_cast<unsigned char*>(std::any_cast<unsigned int>(&item.data));
								dataSize = sizeof(unsigned int);
								break;
							case 2:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::uvec2>(item.data))));
								dataSize = sizeof(glm::uvec2);
								break;
							case 3:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::uvec3>(item.data))));
								dataSize = sizeof(glm::uvec3);
								break;
							case 4:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::uvec4>(item.data))));
								dataSize = sizeof(glm::uvec4);
								break;
						}
						break;
					case SpvType::Float:
						switch(dims) {
							case 1:
								data = reinterpret_cast<unsigned char*>(std::any_cast<float>(&item.data));
								dataSize = sizeof(float);
								break;
							case 2:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::vec2>(item.data))));
								dataSize = sizeof(glm::vec2);
								break;
							case 3:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::vec3>(item.data))));
								dataSize = sizeof(glm::vec3);
								break;
							case 4:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::vec4>(item.data))));
								dataSize = sizeof(glm::vec4);
								break;
							case 6:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::mat2>(item.data))));
								dataSize = sizeof(glm::mat2);
								break;
							case 7:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::mat2x3>(item.data))));
								dataSize = sizeof(glm::mat2x3);
								break;
							case 8:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::mat2x4>(item.data))));
								dataSize = sizeof(glm::mat2x4);
								break;
							case 10:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::mat3x2>(item.data))));
								dataSize = sizeof(glm::mat3x2);
								break;
							case 11:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::mat3>(item.data))));
								dataSize = sizeof(glm::mat3);
								break;
							case 12:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::mat3x4>(item.data))));
								dataSize = sizeof(glm::mat3x4);
								break;
							case 14:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::mat4x2>(item.data))));
								dataSize = sizeof(glm::mat4x2);
								break;
							case 15:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::mat4x3>(item.data))));
								dataSize = sizeof(glm::mat4x3);
								break;
							case 16:
								data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(glm::value_ptr(std::any_cast<glm::mat4>(item.data))));
								dataSize = sizeof(glm::mat4);
								break;
						}
						break;
				}
			} catch(const std::bad_cast&) {
				CheckException(false, Exception::GetExceptionCodeFromMeaning("WrongType"), "Failed cast of shader upload value to type specified in target!");
			}


			//Copy data into buffer if we need to
			if(nd->shaderDataSize > 0) std::memcpy(nd->shaderData + offset, data, dataSize);
		}

		//Copy transformation matrix
		std::memcpy(nd->shaderData, glm::value_ptr(transformation), sizeof(glm::mat4));

		//Push constants if there are any
		activeFrame->cmd.pushConstants(nd->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, nd->shaderDataSize + sizeof(glm::mat4), nd->shaderData);
	}
}