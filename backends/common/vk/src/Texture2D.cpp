#include "Graphics/Textures/Texture2D.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "VkTexture2DData.hpp"
#include "VkShaderData.hpp"
#include "VulkanCoreObjects.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"

#include "stb_image.h"

#include <future>
#include <filesystem>

namespace Cacao {
	Texture2D::Texture2D(std::string filePath)
	  : Texture(false) {
		//Create native data
		nativeData.reset(new Tex2DData());

		//Load image
		dataBuffer = stbi_load(filePath.c_str(), &imgSize.x, &imgSize.y, &numImgChannels, 0);

		CheckException(dataBuffer, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to load 2D texture image file!")

		bound = false;
		currentSlot = -1;

		//Determine image format
		if(numImgChannels == 1) {
			nativeData->format = vk::Format::eR8Unorm;
		} else if(numImgChannels == 3) {
			nativeData->format = vk::Format::eR8G8B8Srgb;
		} else if(numImgChannels == 4) {
			nativeData->format = vk::Format::eR8G8B8A8Srgb;
		}
	}

	std::shared_future<void> Texture2D::Compile() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled texture!")
		const auto doCompile = [this]() {
			//Allocate texture and upload buffer
			vk::ImageCreateInfo texCI({}, vk::ImageType::e2D, nativeData->format, {(unsigned int)imgSize.x, (unsigned int)imgSize.y, 1}, 1, 1, vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, 0);
			vma::AllocationCreateInfo texAllocCI({}, vma::MemoryUsage::eGpuOnly, vk::MemoryPropertyFlagBits::eDeviceLocal);
			vk::BufferCreateInfo uploadCI({}, imgSize.x * imgSize.y * numImgChannels, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
			vma::AllocationCreateInfo uploadAllocCI({}, vma::MemoryUsage::eCpuToGpu);
			auto [image, ialloc] = allocator.createImage(texCI, texAllocCI);
			auto [upload, ualloc] = allocator.createBuffer(uploadCI, uploadAllocCI);

			//Transfer data to upload buffer
			void* gpuMem;
			allocator.mapMemory(ualloc, &gpuMem);
			std::memcpy(gpuMem, dataBuffer, imgSize.x * imgSize.y * numImgChannels);
			allocator.unmapMemory(ualloc);

			//Record a resource copy from the upload buffers to the real buffers
			Immediate imm = immediates[std::this_thread::get_id()];
			vk::CommandBufferBeginInfo copyBegin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
			imm.cmd.begin(copyBegin);
			{
				vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
					vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
					vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0, 0, image, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
				vk::DependencyInfo cdDI({}, {}, {}, barrier);
				imm.cmd.pipelineBarrier2(cdDI);
			}
			{
				vk::BufferImageCopy2 copy(0UL, 0, 0, {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {0}, {(unsigned int)imgSize.x, (unsigned int)imgSize.y, 1});
				vk::CopyBufferToImageInfo2 copyInfo(upload, image, vk::ImageLayout::eTransferDstOptimal, copy);
				imm.cmd.copyBufferToImage2(copyInfo);
			}
			{
				vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
					vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
					vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, image, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
				vk::DependencyInfo cdDI({}, {}, {}, barrier);
				imm.cmd.pipelineBarrier2(cdDI);
			}
			imm.cmd.end();

			//Wait for and reset fence just in case
			if(dev.getFenceStatus(imm.fence) == vk::Result::eSuccess) {
				vk::Result fenceWait = dev.waitForFences(imm.fence, VK_TRUE, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1000)).count());
			CheckException(fenceWait == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("WaitExpired"), "Waited too long for immediate fence reset!")
				dev.resetFences(imm.fence);
			}

			//Submit and wait
			vk::CommandBufferSubmitInfo cbsi(imm.cmd);
			vk::SubmitInfo2 si({}, {}, cbsi);
			immediateQueue.submit2(si, imm.fence);
			dev.waitForFences(imm.fence, VK_TRUE, INFINITY);

			//Assign to native data
			nativeData->texture.alloc = ialloc;
			nativeData->texture.obj = image;

			//Destroy upload buffer
			allocator.destroyBuffer(upload, ualloc);

			//Create image view
			vk::ImageViewCreateInfo viewCI({}, nativeData->texture.obj, vk::ImageViewType::e2D, nativeData->format,
				{vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity},
				{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
			nativeData->iview = dev.createImageView(viewCI);

			//Create sampler
			vk::SamplerCreateInfo samplerCI({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
				vk::SamplerAddressMode::eRepeat, 0.0f, VK_FALSE, 0.0f, VK_FALSE, vk::CompareOp::eNever, 0.0f, VK_REMAINING_MIP_LEVELS, vk::BorderColor::eIntTransparentBlack, VK_FALSE);
			nativeData->sampler = dev.createSampler(samplerCI);

			compiled = true;
		};
		return Engine::GetInstance()->GetThreadPool()->enqueue(doCompile).share();
	}

	void Texture2D::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled texture!")

		//Destroy objects
		dev.destroySampler(nativeData->sampler);
		dev.destroyImageView(nativeData->iview);
		allocator.destroyImage(nativeData->texture.obj, nativeData->texture.alloc);

		compiled = false;
	}

	void Texture2D::Bind(int slot) {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled texture!")
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound texture!")
		CheckException(activeShader, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot bind texture when there is no bound shader!")
		CheckException(std::find(activeShader->imageSlots.begin(), activeShader->imageSlots.end(), slot) != activeShader->imageSlots.end(),
			Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Requested texture slot does not exist in bound shader!")

		//Create update info
		vk::DescriptorImageInfo dii(nativeData->sampler, nativeData->iview, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet wds(activeShader->dset, slot, 0, vk::DescriptorType::eCombinedImageSampler, dii);

		//Update descriptor set
		dev.updateDescriptorSets(wds, {});

		currentSlot = slot;
		nativeData->boundDS = &activeShader->dset;
		bound = true;
	}

	void Texture2D::Unbind() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot unbind uncompiled texture!")
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound texture!")

		//Check if bound descriptor set still exists
		if(!nativeData->boundDS || (nativeData->boundDS && *(nativeData->boundDS))) {
			bound = false;
			return;
		}

		//Create update info
		vk::DescriptorImageInfo dii(VK_NULL_HANDLE, VK_NULL_HANDLE, vk::ImageLayout::eUndefined);
		vk::WriteDescriptorSet wds(*(nativeData->boundDS), currentSlot, 0, vk::DescriptorType::eCombinedImageSampler, dii);

		//Update descriptor set
		dev.updateDescriptorSets(wds, {});

		currentSlot = -1;
		bound = false;
		nativeData->boundDS = nullptr;
	}
}