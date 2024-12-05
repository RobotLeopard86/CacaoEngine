#include "3D/Skybox.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "VkSkyboxData.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"
#include "VulkanCoreObjects.hpp"

#include <future>

#include "vulkan/vulkan.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"

namespace Cacao {
	//Initialize static resources
	bool Skybox::isSetup = false;
	Shader* Skybox::skyboxShader = nullptr;

	Skybox::Skybox(Cubemap* tex)
	  : Asset(false), rotation({0, 0, 0}), textureOwner(true), texture(tex) {
		//Create native data
		nativeData.reset(new SkyboxData());
		nativeData->vbufReady = false;
	}

	void Skybox::_InitCopyND() {
		//Create native data
		nativeData.reset(new SkyboxData());
		nativeData->vbufReady = false;
	}

	void Skybox::CommonSetup() {
		CheckException(!isSetup, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot set up skybox resources that are already set up!");

		//Define skybox shader specification
		ShaderSpec spec;
		ShaderItemInfo skySamplerInfo;
		skySamplerInfo.entryName = "skybox";
		skySamplerInfo.size = {1, 1};
		skySamplerInfo.type = SpvType::SampledImage;
		spec.push_back(skySamplerInfo);

		//Create temporary data objects
		std::vector<uint32_t> v(vsCode, std::end(vsCode));
		std::vector<uint32_t> f(fsCode, std::end(fsCode));

		//Create and compile skybox shader object
		compileMode = ShaderCompileMode::VertexOnly;
		skyboxShader = new Shader(v, f, spec);
		skyboxShader->CompileSync();
		compileMode = ShaderCompileMode::Standard;

		isSetup = true;
	}

	void Skybox::CommonCleanup() {
		CheckException(isSetup, Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot clean up skybox resources that are not set up!");

		skyboxShader->Release();
		delete skyboxShader;

		isSetup = false;
	}

	void Skybox::Draw(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
		//Confirm that texture is compiled
		CheckException(texture->IsCompiled(), Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Skybox texture has not been compiled!");

		//Get frame object
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot draw skybox when there is no active frame object!");
		VkFrame f = *activeFrame;

		//If the vertex buffer isn't set up, do so
		if(!nativeData->vbufReady) {
			//Create allocation info
			auto vbsz = sizeof(float) * std::size(skyboxVerts);
			vk::BufferCreateInfo vertexCI({}, vbsz, vk::BufferUsageFlagBits::eVertexBuffer);
			vma::AllocationCreateInfo allocCI{};
			allocCI.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible;

			//Create buffer object
			{
				auto [buffer, alloc] = allocator.createBuffer(vertexCI, allocCI);
				nativeData->vertexBuffer.alloc = alloc;
				nativeData->vertexBuffer.obj = buffer;
			}

			//Upload data to the GPU
			void* gpuMem;
			allocator.mapMemory(nativeData->vertexBuffer.alloc, &gpuMem);
			std::memcpy(gpuMem, skyboxVerts, vbsz);
			allocator.unmapMemory(nativeData->vertexBuffer.alloc);

			nativeData->vbufReady = true;
		}

		//Create skybox transform matrix
		glm::mat4 skyTransform(1.0);
		skyTransform = glm::rotate(skyTransform, glm::radians(rotation.x), {1.0, 0.0, 0.0});
		skyTransform = glm::rotate(skyTransform, glm::radians(rotation.y), {0.0, 1.0, 0.0});
		skyTransform = glm::rotate(skyTransform, glm::radians(rotation.z), {0.0, 0.0, 1.0});

		//Bind skybox shader
		skyboxShader->Bind();

		//Upload data to shader
		ShaderUploadData sud;
		ShaderUploadItem skySampler;
		skySampler.data = std::any(texture);
		skySampler.target = "skybox";
		sud.push_back(skySampler);
		skyboxShader->UploadData(sud, skyTransform);

		//Bind vertex buffer
		constexpr std::array<vk::DeviceSize, 1> offsets = {{0}};
		f.cmd.bindVertexBuffers(0, nativeData->vertexBuffer.obj, offsets);

		//Draw skybox
		f.cmd.setDepthCompareOp(vk::CompareOp::eLessOrEqual);
		f.cmd.draw(std::size(skyboxVerts), 1, 0, 0);

		//Unbind skybox shader and texture
		texture->Unbind();
		skyboxShader->Unbind();
	}
}