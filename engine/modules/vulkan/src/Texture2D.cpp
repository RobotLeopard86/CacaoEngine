#include "Graphics/Textures/Texture2D.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "VkTexture2DData.hpp"
#include "VkShader.hpp"
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
		stbi_set_flip_vertically_on_load(true);
		dataBuffer = stbi_load(filePath.c_str(), &imgSize.x, &imgSize.y, &numImgChannels, 0);

		CheckException(dataBuffer, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to load 2D texture image file!");

		bound = false;
		currentSlot = -1;

		//Determine image format
		if(numImgChannels == 1) {
			nativeData->format = vk::Format::eR8Unorm;
		} else if(auto has3 = numImgChannels == 3; has3 || numImgChannels == 4) {
			nativeData->format = vk::Format::eR8G8B8A8Srgb;

			//We have to reload the data buffer to have 4 channels now
			if(has3) {
				stbi_image_free(dataBuffer);
				dataBuffer = stbi_load(filePath.c_str(), &imgSize.x, &imgSize.y, &numImgChannels, 4);
				numImgChannels = 4;
				CheckException(dataBuffer, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to load 2D texture image file!");
			}
		}
	}

	std::shared_future<void> Texture2D::CompileAsync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled texture!");
		return Engine::Get()->GetThreadPool()->enqueue([this]() { this->CompileSync(); }).share();
	}

	void Texture2D::CompileSync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled texture!");
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
		Immediate imm = Immediate::Get();
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
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, image, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
			vk::DependencyInfo cdDI({}, {}, {}, barrier);
			imm.cmd.pipelineBarrier2(cdDI);
		}
		imm.cmd.end();

		//Wait for and reset fence just in case
		if(dev.getFenceStatus(imm.fence) == vk::Result::eSuccess) {
			vk::Result fenceWait = dev.waitForFences(imm.fence, VK_TRUE, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1000)).count());
			CheckException(fenceWait == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("WaitExpired"), "Waited too long for immediate fence reset!");
			dev.resetFences(imm.fence);
		}

		//Submit and wait
		vk::CommandBufferSubmitInfo cbsi(imm.cmd);
		vk::SubmitInfo2 si({}, {}, cbsi);
		SubmitCommandBuffer(si, imm.fence);
		dev.waitForFences(imm.fence, VK_TRUE, UINT64_MAX);

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

		compiled = true;
	}

	void Texture2D::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled texture!");

		//Destroy objects
		dev.destroyImageView(nativeData->iview);
		allocator.destroyImage(nativeData->texture.obj, nativeData->texture.alloc);

		compiled = false;
	}

	void Texture2D::Bind(int slot) {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled texture!");
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound texture!");
		CheckException(activeShader, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot bind texture when there is no bound shader!");
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot bind texture when there is no active frame!");
		auto imageSlot = std::find_if(activeShader->imageSlots.begin(), activeShader->imageSlots.end(), [slot](auto is) { return is.second.binding == slot; });
		CheckException(imageSlot != activeShader->imageSlots.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Requested texture slot does not exist in bound shader!");

		//Create update info
		vk::DescriptorImageInfo dii(imageSlot->second.sampler, nativeData->iview, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet wds(VK_NULL_HANDLE, slot, 0, vk::DescriptorType::eCombinedImageSampler, dii);

		//Update descriptor set
		activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);

		currentSlot = slot;
		bound = true;
	}

	void Texture2D::Unbind() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot unbind uncompiled texture!");
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound texture!");
		CheckException(activeShader, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot unbind texture when there is no bound shader!");
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot unbind texture when there is no active frame!");
		auto imageSlot = std::find_if(activeShader->imageSlots.begin(), activeShader->imageSlots.end(), [this](auto is) { return is.second.binding == currentSlot; });

		//Create update info
		vk::DescriptorImageInfo dii(imageSlot->second.sampler, nullView, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet wds(VK_NULL_HANDLE, currentSlot, 0, vk::DescriptorType::eCombinedImageSampler, dii);

		//Update descriptor set
		activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);

		currentSlot = -1;
		bound = false;
	}
}