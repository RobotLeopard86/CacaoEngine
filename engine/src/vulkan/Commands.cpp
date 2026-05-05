#include "VulkanModule.hpp"

namespace Cacao {
	void VulkanCommandBuffer::StartRendering(glm::vec3 clearColor) {
		//Setup rendering info
		vk::Viewport viewport(0.0f, 0.0f, float(vulkan->swapchain.extent.width), float(vulkan->swapchain.extent.height), 0.0f, 1.0f);
		vk::Rect2D scissor({0, 0}, vulkan->swapchain.extent);
		vk::RenderingAttachmentInfo colorAttachment(vulkan->swapchain.views[render2->imageIndex], vk::ImageLayout::eColorAttachmentOptimal, {}, {}, {},
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearColorValue(std::array<float, 4> {clearColor.r, clearColor.g, clearColor.b, 1.0f}));
		vk::RenderingAttachmentInfo depthAttachment(vulkan->swapchain.depthImages[render2->imageIndex].view, vk::ImageLayout::eDepthAttachmentOptimal, {}, {}, {},
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearDepthStencilValue(1.0f, 0.0f));
		vk::RenderingInfo renderInfo(vk::RenderingFlagBits::eContentsSecondaryCommandBuffers, vk::Rect2D({0, 0}, vulkan->swapchain.extent), 1, 0, colorAttachment, &depthAttachment);

		//Make our image drawable
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllGraphics, vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eAllGraphics, vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 0, 0, vulkan->swapchain.images[render2->imageIndex],
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			cmd.pipelineBarrier2(transition);
		}

		//Make the depth image drawable
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllGraphics, vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eAllGraphics, vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, 0, 0, vulkan->swapchain.depthImages[render2->imageIndex].obj,
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			cmd.pipelineBarrier2(transition);
		}

		//Configure output region
		cmd.setViewport(0, viewport);
		cmd.setScissor(0, scissor);

		//Begin rendering (this will clear the screen due to values set above)
		cmd.beginRendering(renderInfo);
	}

	void VulkanCommandBuffer::EndRendering() {
		cmd.endRendering();
	}
}