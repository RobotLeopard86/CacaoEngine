#include "VulkanTex2D.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "VulkanModule.hpp"

namespace Cacao {
	void VulkanTex2DImpl::Realize(bool& success) {
		//Get texture format
		switch(img.layout) {
			case libcacaoimage::Image::Layout::Grayscale:
				format = vk::Format::eR8Srgb;
				break;
			case libcacaoimage::Image::Layout::RGB:
				format = vk::Format::eR8G8B8Srgb;
				break;
			case libcacaoimage::Image::Layout::RGBA:
				format = vk::Format::eR8G8B8A8Srgb;
				break;
		}

		//Allocate GPU texture & data upload buffers
		vk::ImageCreateInfo texCI({}, vk::ImageType::e2D, format, {img.w, img.h, 1}, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, 0);
		vma::AllocationCreateInfo texAllocCI(vma::AllocationCreateFlagBits::eWithinBudget, vma::MemoryUsage::eAutoPreferDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vk::BufferCreateInfo texUpCI({}, img.w * img.h * static_cast<uint8_t>(img.layout), vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, 0);
		vma::AllocationCreateInfo texUpAllocCI(vma::AllocationCreateFlagBits::eWithinBudget | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite, vma::MemoryUsage::eAuto,
			vk::MemoryPropertyFlagBits::eHostVisible);
		Allocated<vk::Buffer> up;
		try {
			vi = vulkan->allocator.createImage(texCI, texAllocCI);
			up = vulkan->allocator.createBuffer(texUpCI, texUpAllocCI);
		} catch(const vk::SystemError& vkse) {
			std::stringstream msg;
			msg << "Encountered Vulkan exception during texture buffer creation: " << vkse.what();
			Check<ExternalException>(false, msg.str());
		}

		//Copy image data to upload buffer
		void* gpuMem;
		Check<ExternalException>(vulkan->allocator.mapMemory(up.alloc, &gpuMem) == vk::Result::eSuccess, "Failed to map texture upload buffer memory!");
		std::memcpy(gpuMem, img.data.data(), img.w * img.h * static_cast<uint8_t>(img.layout));
		vulkan->allocator.unmapMemory(up.alloc);

		//Transfer data from upload buffer to real texture memory
		std::unique_ptr<VulkanCommandBuffer> vcb = std::make_unique<VulkanCommandBuffer>();
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0, 0, vi.obj, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
			vk::DependencyInfo cdDI({}, {}, {}, barrier);
			vcb->vk().pipelineBarrier2(cdDI);
		}
		{
			vk::BufferImageCopy2 copy(0UL, 0, 0, {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {0}, {img.w, img.h, 1});
			vk::CopyBufferToImageInfo2 copyInfo(up.obj, vi.obj, vk::ImageLayout::eTransferDstOptimal, copy);
			vcb->vk().copyBufferToImage2(copyInfo);
		}
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, vi.obj, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
			vk::DependencyInfo cdDI({}, {}, {}, barrier);
			vcb->vk().pipelineBarrier2(cdDI);
		}
		GPUManager::Get().Submit(std::move(vcb)).get();

		//Destroy upload buffer
		vulkan->allocator.destroyBuffer(up.obj, up.alloc);

		//Create image view
		vk::ImageViewCreateInfo viewCI({}, vi.obj, vk::ImageViewType::e2D, format,
			{vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity},
			{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
		vi.view = vulkan->dev.createImageView(viewCI);

		success = true;
	}

	void VulkanTex2DImpl::DropRealized() {
		vulkan->dev.destroyImageView(vi.view);
		vulkan->allocator.destroyImage(vi.obj, vi.alloc);
	}

	Tex2D::Impl* VulkanModule::ConfigureTex2D() {
		return new VulkanTex2DImpl();
	}
}