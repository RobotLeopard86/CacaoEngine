#include "Graphics/Textures/Cubemap.hpp"

#include "Core/Log.hpp"
#include "Core/Engine.hpp"
#include "Core/Exception.hpp"
#include "VkCubemapData.hpp"
#include "VkShaderData.hpp"
#include "VulkanCoreObjects.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"

#include "stb_image.h"

#include <future>
#include <filesystem>

namespace Cacao {
	Cubemap::Cubemap(std::vector<std::string> filePaths)
	  : Texture(false) {
		//Create native data
		nativeData.reset(new CubemapData());

		for(std::string tex : filePaths) {
			CheckException(std::filesystem::exists(tex), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create cubemap from nonexistent file!")
			;
		}

		textures = filePaths;
		currentSlot = -1;
	}

	std::shared_future<void> Cubemap::Compile() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled cubemap!")
		const auto doCompile = [this]() {
			//Load images
			glm::uvec2 imgSize = {0, 0};
			std::array<unsigned char*, 6> faces;
			for(unsigned int i = 0; i < textures.size(); i++) {
				int w, h, _;

				//Load texture data from file
				unsigned char* data = stbi_load(textures[i].c_str(), &w, &h, &_, 4);
				if(data) {
					if(imgSize.x == 0 || imgSize.y == 0) imgSize = {w, h};
			CheckException(w == imgSize.x && h == imgSize.y, Exception::GetExceptionCodeFromMeaning("BadValue"), "All cubemap faces must be the same size!")

					faces[i] = data;
				} else {
					//Free whatever junk we have
					stbi_image_free(data);

			CheckException(false, Exception::GetExceptionCodeFromMeaning("IO"), "Failed to open cubemap face image file!")
				}
			}

			vk::DeviceSize faceSize = imgSize.x * imgSize.y * 4;
			vk::DeviceSize totalSize = faceSize * 6;

			//Allocate texture and upload buffer
			vk::ImageCreateInfo texCI(vk::ImageCreateFlagBits::eCubeCompatible, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb, {imgSize.x, imgSize.y, 1}, 1, 6, vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, 0);
			vma::AllocationCreateInfo texAllocCI({}, vma::MemoryUsage::eGpuOnly, vk::MemoryPropertyFlagBits::eDeviceLocal);
			vk::BufferCreateInfo uploadCI({}, totalSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
			vma::AllocationCreateInfo uploadAllocCI({}, vma::MemoryUsage::eCpuToGpu);
			auto [image, ialloc] = allocator.createImage(texCI, texAllocCI);
			auto [upload, ualloc] = allocator.createBuffer(uploadCI, uploadAllocCI);

			//Make cubemap data contiguous
			std::vector<unsigned char> contiguous(totalSize);
			for(int i = 0; i < faces.size(); i++) {
				std::memcpy(&contiguous[i * faceSize], faces[i], faceSize);
			}

			//Transfer data to upload buffer
			void* gpuMem;
			allocator.mapMemory(ualloc, &gpuMem);
			std::memcpy(gpuMem, contiguous.data(), totalSize);
			allocator.unmapMemory(ualloc);

			//Free image data
			for(unsigned char* f : faces) {
				stbi_image_free(f);
			}

			//Record a resource copy from the upload buffers to the real buffers
			Immediate imm = immediates[std::this_thread::get_id()];
			vk::CommandBufferBeginInfo copyBegin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
			imm.cmd.begin(copyBegin);
			{
				vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
					vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
					vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0, 0, image, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6});
				vk::DependencyInfo cdDI({}, {}, {}, barrier);
				imm.cmd.pipelineBarrier2(cdDI);
			}
			{
				std::vector<vk::BufferImageCopy2> copies;
				for(unsigned int i = 0; i < faces.size(); i++) {
					vk::BufferImageCopy2 copy(i * faceSize, imgSize.x, imgSize.y, {vk::ImageAspectFlagBits::eColor, 0, i, 1}, {0}, {imgSize.x, imgSize.y, 1});
					copies.push_back(copy);
				}
				vk::CopyBufferToImageInfo2 copyInfo(upload, image, vk::ImageLayout::eTransferDstOptimal, copies);
				imm.cmd.copyBufferToImage2(copyInfo);
			}
			{
				vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
					vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
					vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, image, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6});
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
			imm.queue.submit2(si, imm.fence);
			dev.waitForFences(imm.fence, VK_TRUE, INFINITY);

			//Assign to native data
			nativeData->texture.alloc = ialloc;
			nativeData->texture.obj = image;

			//Destroy upload buffer
			allocator.destroyBuffer(upload, ualloc);

			//Create image view
			vk::ImageViewCreateInfo viewCI({}, nativeData->texture.obj, vk::ImageViewType::eCube, vk::Format::eR8G8B8A8Srgb,
				{vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eOne},
				{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6});
			nativeData->iview = dev.createImageView(viewCI);

			compiled = true;
		};
		return Engine::GetInstance()->GetThreadPool()->enqueue(doCompile).share();
	}

	void Cubemap::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled cubemap!")
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release bound cubemap!")

		//Destroy objects
		dev.destroyImageView(nativeData->iview);
		allocator.destroyImage(nativeData->texture.obj, nativeData->texture.alloc);

		compiled = false;
	}

	void Cubemap::Bind(int slot) {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind uncompiled cubemap!")
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound cubemap!")
		CheckException(activeShader, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot bind cubemap when there is no bound shader!")
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot bind cubemap when there is no active frame!")
		auto imageSlot = std::find_if(activeShader->imageSlots.begin(), activeShader->imageSlots.end(), [slot](auto is) { return is.second.binding == slot; });
		CheckException(imageSlot != activeShader->imageSlots.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Requested texture slot does not exist in bound shader!")

		//Create update info
		vk::DescriptorImageInfo dii(imageSlot->second.sampler, nativeData->iview, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet wds(VK_NULL_HANDLE, slot, 0, vk::DescriptorType::eCombinedImageSampler, dii);

		//Update descriptor set
		activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);

		currentSlot = slot;
		bound = true;
	}

	void Cubemap::Unbind() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot unbind uncompiled cubemap!")
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound cubemap!")
		CheckException(activeShader, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot unbind cubemap when there is no bound shader!")
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot unbind cubemap when there is no active frame!")
		auto imageSlot = std::find_if(activeShader->imageSlots.begin(), activeShader->imageSlots.end(), [this](auto is) { return is.second.binding == currentSlot; });

		//Create update info
		vk::DescriptorImageInfo dii(imageSlot->second.sampler, VK_NULL_HANDLE, vk::ImageLayout::eUndefined);
		vk::WriteDescriptorSet wds(VK_NULL_HANDLE, currentSlot, 0, vk::DescriptorType::eCombinedImageSampler, dii);

		//Update descriptor set
		activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);

		currentSlot = -1;
		bound = false;
	}
}