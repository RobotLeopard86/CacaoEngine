#include "VulkanCubemap.hpp"
#include "Cacao/Engine.hpp"
#include "VulkanModule.hpp"

namespace Cacao {
	void VulkanCubemapImpl::Realize(bool& success) {
		//Calculate sizes
		vk::DeviceSize faceSize = faces[0].w * faces[0].h * 3;
		vk::DeviceSize totalSize = faceSize * 6;

		//Allocate GPU texture & data upload buffers
		vk::ImageCreateInfo texCI(vk::ImageCreateFlagBits::eCubeCompatible, vk::ImageType::e2D, vk::Format::eR8G8B8Srgb, {faces[0].w, faces[0].h, 1}, 1, 1,
			vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, 0);
		vma::AllocationCreateInfo texAllocCI(vma::AllocationCreateFlagBits::eWithinBudget, vma::MemoryUsage::eAutoPreferDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vk::BufferCreateInfo texUpCI({}, totalSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, 0);
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

		//Make cubemap data contiguous
		std::vector<unsigned char> contiguous(totalSize);
		for(unsigned short i = 0; i < faces.size(); i++) {
			std::memcpy(&contiguous[i * faceSize], faces[i].data.data(), faceSize);
		}

		//Copy image data to upload buffer
		void* gpuMem;
		Check<ExternalException>(vulkan->allocator.mapMemory(up.alloc, &gpuMem) == vk::Result::eSuccess, "Failed to map texture upload buffer memory!");
		std::memcpy(gpuMem, contiguous.data(), totalSize);
		vulkan->allocator.unmapMemory(up.alloc);

		//Transfer data from upload buffer to real texture memory
		std::unique_ptr<VulkanCommandBuffer> vcb = std::make_unique<VulkanCommandBuffer>();
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0, 0, vi.obj, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6});
			vk::DependencyInfo cdDI({}, {}, {}, barrier);
			vcb->cmd.pipelineBarrier2(cdDI);
		}
		{
			std::vector<vk::BufferImageCopy2> copies;
			for(unsigned int i = 0; i < faces.size(); i++) {
				vk::BufferImageCopy2 copy(i * faceSize, faces[0].w, faces[0].h, {vk::ImageAspectFlagBits::eColor, 0, i, 1}, {0}, {faces[0].w, faces[0].h, 1});
				copies.push_back(copy);
			}
			vk::CopyBufferToImageInfo2 copyInfo(up.obj, vi.obj, vk::ImageLayout::eTransferDstOptimal, copies);
			vcb->cmd.copyBufferToImage2(copyInfo);
		}
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, vi.obj, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6});
			vk::DependencyInfo cdDI({}, {}, {}, barrier);
			vcb->cmd.pipelineBarrier2(cdDI);
		}
		GPUManager::Get().Submit(std::move(vcb)).get();

		//Destroy upload buffer
		vulkan->allocator.destroyBuffer(up.obj, up.alloc);

		//Create image view
		vk::ImageViewCreateInfo viewCI({}, vi.obj, vk::ImageViewType::eCube, vk::Format::eR8G8B8Srgb,
			{vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eOne},
			{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6});
		vi.view = vulkan->dev.createImageView(viewCI);

		success = true;
	}

	void VulkanCubemapImpl::DropRealized() {
		vulkan->dev.destroyImageView(vi.view);
		vulkan->allocator.destroyImage(vi.obj, vi.alloc);
	}

	Cubemap::Impl* VulkanModule::ConfigureCubemap() {
		return new VulkanCubemapImpl();
	}
}