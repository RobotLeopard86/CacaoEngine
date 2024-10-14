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

//See VkShaderData.hpp for why there's this "impl" thing
//Basically this just makes it easier to use and keeps the arrow operator method of
//accessing native data intact
#define nd (&(this->nativeData->impl))

namespace Cacao {
	//Calculates shader data offsets and image slots
	void GetShaderUniformInfo(const ShaderSpec& spec, VkShaderData* mod) {
		CheckException(mod, Exception::GetExceptionCodeFromMeaning("NullValue"), "Passed-in shader data pointer is invalid!")

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
		spirv_cross::ShaderResources fr = vertReflector.get_shader_resources();

		//Get shader data block size and offsets
		for(auto pcb : vr.push_constant_buffers) {
			if(pcb.name.compare("type.PushConstant.ShaderData") == 0 || pcb.name.compare("shader") == 0) {
				spirv_cross::SPIRType type = vertReflector.get_type(pcb.base_type_id);
				mod->shaderDataSize = vertReflector.get_declared_struct_size(type);
				for(unsigned int i = 0; i < type.member_types.size(); ++i) {
					mod->offsets.insert_or_assign(vertReflector.get_member_name(pcb.base_type_id, i), vertReflector.type_struct_member_offset(type, i));
				}
				break;
			}
		}

		//Get valid image slots
		for(auto tex : fr.sampled_images) {
			mod->imageSlots.insert_or_assign(fragReflector.get_name(tex.id), fragReflector.get_decoration(tex.id, spv::DecorationBinding));
		}

		//Make sure that shader data matches spec
		for(const ShaderItemInfo& sii : spec) {
			CheckException((sii.type == SpvType::SampledImage && mod->imageSlots.contains(sii.entryName)) || mod->offsets.contains(sii.entryName), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Value found in shader spec that is not present in shader!")
		}
		for(auto offset : mod->offsets) {
			CheckException(std::find_if(spec.begin(), spec.end(), [&offset](const ShaderItemInfo& sii) { return offset.first.compare(sii.entryName) == 0; }) != spec.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Value found in shader that is not in shader spec!")
		}
		for(auto slot : mod->imageSlots) {
			CheckException(std::find_if(spec.begin(), spec.end(), [&slot](const ShaderItemInfo& sii) { return slot.first.compare(sii.entryName) == 0 && sii.type == SpvType::SampledImage; }) != spec.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Texture found in shader that is not in shader spec!")
		}
	}

	Shader::Shader(std::string vertexPath, std::string fragmentPath, ShaderSpec spec)
	  : Asset(false), bound(false), specification(spec) {
		//Validate that these paths exist
		CheckException(std::filesystem::exists(vertexPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create a shader from a non-existent vertex shader file!")
		CheckException(std::filesystem::exists(fragmentPath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create a shader from a non-existent fragment shader file!")

		//Load SPIR-V code

		//Open file streams
		FILE* vf = fopen(vertexPath.c_str(), "rb");
		CheckException(vf, Exception::GetExceptionCodeFromMeaning("FileOpenFailure"), "Failed to open vertex shader file!")
		FILE* ff = fopen(fragmentPath.c_str(), "rb");
		CheckException(ff, Exception::GetExceptionCodeFromMeaning("FileOpenFailure"), "Failed to open fragment shader file!")

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
			CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to read vertex shader data from file!")
		}
		if(fread(fbuf.data(), sizeof(uint32_t), flen, ff) != std::size_t(flen)) {
			//Clean up file streams before exception throw
			fclose(vf);
			fclose(ff);
			CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to read fragment shader data from file!")
		}

		//Close file streams
		fclose(vf);
		fclose(ff);

		//Create native data
		nativeData.reset(new ShaderData());
		nd->vertexCode = vbuf;
		nd->fragmentCode = fbuf;

		//Get shader uniform info
		GetShaderUniformInfo(spec, nd);
	}

	Shader::Shader(std::vector<uint32_t>& vertex, std::vector<uint32_t>& fragment, ShaderSpec spec)
	  : Asset(false), bound(false), specification(spec) {
		//Create native data
		nativeData.reset(new ShaderData());

		//Create native data
		nativeData.reset(new ShaderData());
		nd->vertexCode = vertex;
		nd->fragmentCode = fragment;

		//Get shader uniform info
		GetShaderUniformInfo(spec, nd);
	}

	std::shared_future<void> Shader::Compile() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled texture!")
		const auto doCompile = [this]() {
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
				vk::FrontFace::eClockwise, VK_FALSE, 0, 0, 0, 1.0f);
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
				vk::DynamicState::eColorWriteEnableEXT,
				vk::DynamicState::eDepthTestEnable};
			vk::PipelineDynamicStateCreateInfo dynStateCI({}, dynamicStates);

			//Create pipeline rendering info
			vk::PipelineRenderingCreateInfo pipelineRenderingInfo(0, surfaceFormat.format, vk::Format::eD32Sfloat);

			//Create input attributes and bindings
			vk::VertexInputBindingDescription inputBinding(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
			std::array<vk::VertexInputAttributeDescription, 5> inputAttrs {{{0, 0, vk::Format::eR32G32B32Sfloat, 0},
				{1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoords)},
				{2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, tangent)},
				{3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, bitangent)},
				{4, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)}}};
			vk::PipelineVertexInputStateCreateInfo inputStateCI({}, inputBinding, inputAttrs);

			//Create descriptor set layout
			std::vector<vk::DescriptorSetLayoutBinding> dsBindings;
			dsBindings.emplace_back(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, VK_NULL_HANDLE);
			dsBindings.emplace_back(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, VK_NULL_HANDLE);
			for(auto slot : nd->imageSlots) {
				dsBindings.emplace_back(slot.second, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, VK_NULL_HANDLE);
			}
			vk::DescriptorSetLayout dsLayout = dev.createDescriptorSetLayout({{}, dsBindings});

			//Create pipeline layout
			vk::PushConstantRange pcr(vk::ShaderStageFlagBits::eVertex, 0, nd->shaderDataSize);
			vk::PipelineLayoutCreateInfo layoutCI({}, dsLayout, pcr);
			vk::PipelineLayout layout = dev.createPipelineLayout(layoutCI);

			//Create pipeline
			vk::GraphicsPipelineCreateInfo pipelineCI({}, shaderStages, &inputStateCI, &inputAssemblyCI, nullptr, &viewportState, &rasterizerCI,
				&multisamplingCI, &depthStencilCI, &colorBlendCI, &dynStateCI, layout);
			auto pipelineResult = dev.createGraphicsPipeline({}, pipelineCI);
			CheckException(pipelineResult.result == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("Vulkan"), "Failed to create shader pipeline!")
			nd->pipeline = pipelineResult.value;

			//Allocate shader data memory (scary)
			nd->shaderData = (unsigned char*)malloc(nd->shaderDataSize);

			//Create and map locals UBO
			vk::BufferCreateInfo localsCI({}, sizeof(glm::mat4) * 2, vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
			vma::AllocationCreateInfo localsAllocCI({}, vma::MemoryUsage::eCpuToGpu);
			try {
				auto [locals, alloc] = allocator.createBuffer(localsCI, localsAllocCI);
				nd->localsUBO = {.alloc = alloc, .obj = locals};
			} catch(vk::SystemError& err) {
				std::stringstream emsg;
				emsg << "Failed to create locals uniform buffer: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str())
			}
			CheckException(allocator.mapMemory(nd->localsUBO.alloc, &nd->locals) == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("Vulkan"), "Failed to map locals uniform buffer memory!")

			//Create descriptor pool
			std::array<vk::DescriptorPoolSize, 2> poolSizes {{{vk::DescriptorType::eUniformBuffer, 2},
				{vk::DescriptorType::eCombinedImageSampler, (unsigned int)nd->imageSlots.size()}}};
			vk::DescriptorPoolCreateInfo poolCI({}, 1, poolSizes);
			try {
				nd->dpool = dev.createDescriptorPool(poolCI);
			} catch(vk::SystemError& err) {
				std::stringstream emsg;
				emsg << "Failed to create shader descriptor pool: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str())
			}

			//Allocate descriptor set
			vk::DescriptorSetAllocateInfo setAllocInfo(nd->dpool, dsLayout);
			try {
				nd->dset = dev.allocateDescriptorSets(setAllocInfo)[0];
			} catch(vk::SystemError& err) {
				std::stringstream emsg;
				emsg << "Failed to allocate shader descriptor set: " << err.what();
			CheckException(false, Exception::GetExceptionCodeFromMeaning("Vulkan"), emsg.str())
			}

			//Bind globals and locals UBOs
			vk::DescriptorBufferInfo globalsDBI(globalsUBO.obj, 0, vk::WholeSize);
			vk::DescriptorBufferInfo localsDBI(nd->localsUBO.obj, 0, vk::WholeSize);
			std::array<vk::WriteDescriptorSet, 2> dsWrites {{{nd->dset, 0, 0, 1, vk::DescriptorType::eUniformBuffer, VK_NULL_HANDLE, &globalsDBI},
				{nd->dset, 1, 0, 1, vk::DescriptorType::eUniformBuffer, VK_NULL_HANDLE, &localsDBI}}};
			dev.updateDescriptorSets(dsWrites, {});

			compiled = true;
		};
		return Engine::GetInstance()->GetThreadPool()->enqueue(doCompile).share();
	}

	void Shader::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled shader!")
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release bound shader!")

		//Destroy descriptor pool and set
		dev.resetDescriptorPool(nd->dpool);
		dev.destroyDescriptorPool(nd->dpool);

		//Unmap and destroy locals UBO
		allocator.unmapMemory(nd->localsUBO.alloc);
		allocator.destroyBuffer(nd->localsUBO.obj, nd->localsUBO.alloc);

		//Free shader data memory
		free(nd->shaderData);

		//Destroy pipeline
		dev.destroyPipeline(nd->pipeline);

		compiled = false;
	}

	void Shader::Bind() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled shader!")
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound shader!")
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot bind shader when there is no active frame object!")

		//Bind pipeline object
		activeFrame->cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, nd->pipeline);

		bound = true;
	}

	void Shader::Unbind() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot unbind uncompiled shader!")
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound shader!")

		//Vulkan has no concept of unbinding a pipeline, also since command buffers shift around state is wiped clean every time
		bound = false;
	}

	void Shader::UploadCacaoGlobals(glm::mat4 projection, glm::mat4 view) {
		struct CombinedViewProjection {
			glm::mat4 p;
			glm::mat4 v;
		} cvp = {.p = projection, .v = view};
		std::memcpy(globalsMem, &cvp, sizeof(cvp));
	}

	void Shader::UploadCacaoLocals(glm::mat4 transform) {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot uplaod locals data to uncompiled shader!")
		std::memcpy(nd->locals, &transform, sizeof(glm::mat4));
	}

	void Shader::UploadData(ShaderUploadData& data) {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled shader!")
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound shader!")
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot bind shader when there is no active frame object!")

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
			CheckException(found, Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Can't locate item targeted by upload in shader specification!")

			//Grab shader item info
			ShaderItemInfo info = foundItems[item.target];

			//Find offset
			CheckException(nd->offsets.contains(item.target), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Can't locate item targeted by upload in shader offsets!")
			auto offset = nd->offsets[item.target];

			//Turn dimensions into single number (easier for uploading)
			int dims = (4 * info.size.y) - (4 - info.size.x);
			CheckException(!(info.size.x == 1 && info.size.y >= 2), Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Shaders cannot have data with one column and 2+ rows!")
			CheckException(!(info.size.x > 1 && info.size.y > 1 && info.type != SpvType::Float && info.type != SpvType::Double), Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Shaders cannot have data with 2+ columns and rows that are not floats or doubles!")

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
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::ivec2>(&item.data));
								dataSize = sizeof(glm::ivec2);
								break;
							case 3:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::ivec3>(&item.data));
								dataSize = sizeof(glm::ivec3);
								break;
							case 4:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::ivec4>(&item.data));
								dataSize = sizeof(glm::ivec4);
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
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::ivec2>(&item.data));
								dataSize = sizeof(glm::ivec2);
								break;
							case 3:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::ivec3>(&item.data));
								dataSize = sizeof(glm::ivec3);
								break;
							case 4:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::ivec4>(&item.data));
								dataSize = sizeof(glm::ivec4);
								break;
						}
						break;
					case SpvType::SampledImage:
						CheckException(dims == 1, Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Shaders cannot have arrays or matrices of textures!")

						//Bind texture to the slot specified
						if(item.data.type() == typeid(Texture2D*)) {
							Texture2D* tex = std::any_cast<Texture2D*>(item.data);
							tex->Bind(nd->imageSlots[item.target]);
						} else if(item.data.type() == typeid(Cubemap*)) {
							Cubemap* tex = std::any_cast<Cubemap*>(item.data);
							tex->Bind(nd->imageSlots[item.target]);
						} else if(item.data.type() == typeid(UIView*)) {
							UIView* view = std::any_cast<UIView*>(item.data);
							view->Bind(nd->imageSlots[item.target]);
						} else if(item.data.type() == typeid(AssetHandle<Texture2D>)) {
							AssetHandle<Texture2D> tex = std::any_cast<AssetHandle<Texture2D>>(item.data);
							tex->Bind(nd->imageSlots[item.target]);
						} else if(item.data.type() == typeid(AssetHandle<Cubemap>)) {
							AssetHandle<Cubemap> tex = std::any_cast<AssetHandle<Cubemap>>(item.data);
							tex->Bind(nd->imageSlots[item.target]);
						} else if(item.data.type() == typeid(AssetHandle<UIView>)) {
							AssetHandle<UIView> view = std::any_cast<AssetHandle<UIView>>(item.data);
							view->Bind(nd->imageSlots[item.target]);
						} else {
							CheckException(false, Exception::GetExceptionCodeFromMeaning("UniformUploadFailure"), "Non-texture value supplied to texture uniform!")
						}

						break;
					case SpvType::UInt:
						switch(dims) {
							case 1:
								data = reinterpret_cast<unsigned char*>(std::any_cast<unsigned int>(&item.data));
								dataSize = sizeof(unsigned int);
								break;
							case 2:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::uvec2>(&item.data));
								dataSize = sizeof(glm::uvec2);
								break;
							case 3:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::uvec3>(&item.data));
								dataSize = sizeof(glm::uvec3);
								break;
							case 4:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::uvec4>(&item.data));
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
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::vec2>(&item.data));
								dataSize = sizeof(glm::vec2);
								break;
							case 3:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::vec3>(&item.data));
								dataSize = sizeof(glm::vec3);
								break;
							case 4:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::vec4>(&item.data));
								dataSize = sizeof(glm::vec4);
								break;
							case 6:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::mat2>(&item.data));
								dataSize = sizeof(glm::mat2);
								break;
							case 7:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::mat2x3>(&item.data));
								dataSize = sizeof(glm::mat2x3);
								break;
							case 8:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::mat2x4>(&item.data));
								dataSize = sizeof(glm::mat2x4);
								break;
							case 10:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::mat3x2>(&item.data));
								dataSize = sizeof(glm::mat3x2);
								break;
							case 11:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::mat3>(&item.data));
								dataSize = sizeof(glm::mat3);
								break;
							case 12:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::mat3x4>(&item.data));
								dataSize = sizeof(glm::mat3x4);
								break;
							case 14:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::mat4x2>(&item.data));
								dataSize = sizeof(glm::mat4x2);
								break;
							case 15:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::mat4x3>(&item.data));
								dataSize = sizeof(glm::mat4x3);
								break;
							case 16:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::mat4>(&item.data));
								dataSize = sizeof(glm::mat4);
								break;
						}
						break;
					case SpvType::Double:
						switch(dims) {
							case 1:
								data = reinterpret_cast<unsigned char*>(std::any_cast<double>(&item.data));
								dataSize = sizeof(double);
								break;
							case 2:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dvec2>(&item.data));
								dataSize = sizeof(glm::dvec2);
								break;
							case 3:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dvec3>(&item.data));
								dataSize = sizeof(glm::dvec3);
								break;
							case 4:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dvec4>(&item.data));
								dataSize = sizeof(glm::dvec4);
								break;
							case 6:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dmat2>(&item.data));
								dataSize = sizeof(glm::dmat2);
								break;
							case 7:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dmat2x3>(&item.data));
								dataSize = sizeof(glm::dmat2x3);
								break;
							case 8:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dmat2x4>(&item.data));
								dataSize = sizeof(glm::dmat2x4);
								break;
							case 10:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dmat3x2>(&item.data));
								dataSize = sizeof(glm::dmat3x2);
								break;
							case 11:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dmat3>(&item.data));
								dataSize = sizeof(glm::dmat3);
								break;
							case 12:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dmat3x4>(&item.data));
								dataSize = sizeof(glm::dmat3x4);
								break;
							case 14:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dmat4x2>(&item.data));
								dataSize = sizeof(glm::dmat4x2);
								break;
							case 15:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dmat4x3>(&item.data));
								dataSize = sizeof(glm::dmat4x3);
								break;
							case 16:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::dmat4>(&item.data));
								dataSize = sizeof(glm::dmat4);
								break;
						}
						break;
					case SpvType::Int64:
						switch(dims) {
							case 1:
								data = reinterpret_cast<unsigned char*>(std::any_cast<int64_t>(&item.data));
								dataSize = sizeof(int64_t);
								break;
							case 2:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::i64vec2>(&item.data));
								dataSize = sizeof(glm::i64vec2);
								break;
							case 3:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::i64vec3>(&item.data));
								dataSize = sizeof(glm::i64vec3);
								break;
							case 4:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::i64vec4>(&item.data));
								dataSize = sizeof(glm::i64vec4);
								break;
						}
						break;
					case SpvType::UInt64:
						switch(dims) {
							case 1:
								data = reinterpret_cast<unsigned char*>(std::any_cast<uint64_t>(&item.data));
								dataSize = sizeof(uint64_t);
								break;
							case 2:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::u64vec2>(&item.data));
								dataSize = sizeof(glm::u64vec2);
								break;
							case 3:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::u64vec3>(&item.data));
								dataSize = sizeof(glm::u64vec3);
								break;
							case 4:
								data = reinterpret_cast<unsigned char*>(std::any_cast<glm::u64vec4>(&item.data));
								dataSize = sizeof(glm::u64vec4);
								break;
						}
						break;
				}
			} catch(const std::bad_cast&) {
				CheckException(false, Exception::GetExceptionCodeFromMeaning("WrongType"), "Failed cast of shader upload value to type specified in target!")
			}


			//Copy data into buffer
			std::memcpy(nd->shaderData + offset, data, dataSize);
		}
	}
}