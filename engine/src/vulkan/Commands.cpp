#include "VulkanModule.hpp"

namespace Cacao {
	void VulkanCommandBuffer::StartRendering(glm::vec3 clearColor) {
		//NOTE: THIS FUNCTION USES THE PRIMARY COMMAND BUFFER TO SATISY THE REQUIREMENTS OF DYNAMIC RENDERING
		//      THIS SHOULD NOT BE DONE IN ALMOST ALL CASES. USE vk() TO GET A SECONDARY COMMAND BUFFER.

		//Setup rendering info
		vk::Viewport viewport(0.0f, 0.0f, float(vulkan->swapchain.extent.width), float(vulkan->swapchain.extent.height), 0.0f, 1.0f);
		vk::Rect2D scissor({0, 0}, vulkan->swapchain.extent);
		vk::RenderingAttachmentInfo colorAttachment(vulkan->swapchain.views[render->imageIndex], vk::ImageLayout::eColorAttachmentOptimal, {}, {}, {},
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearColorValue(std::array<float, 4> {clearColor.r, clearColor.g, clearColor.b, 1.0f}));
		vk::RenderingAttachmentInfo depthAttachment(vulkan->depth.view, vk::ImageLayout::eDepthAttachmentOptimal, {}, {}, {},
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearDepthStencilValue(1.0f, 0.0f));
		vk::RenderingInfo renderInfo(vk::RenderingFlagBits::eContentsSecondaryCommandBuffers, vk::Rect2D({0, 0}, vulkan->swapchain.extent), 1, 0, colorAttachment, &depthAttachment);

		//Make our image drawable
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 0, 0, vulkan->swapchain.images[render->imageIndex],
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			primary.pipelineBarrier2(transition);
		}

		//Make the depth image drawable
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, 0, 0, vulkan->depth.obj,
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			primary.pipelineBarrier2(transition);
		}

		//Configure output region
		primary.setViewport(0, viewport);
		primary.setScissor(0, scissor);

		//Begin rendering (this will clear the screen due to values set above)
		primary.beginRendering(renderInfo);

		didStartRender = true;
	}

	void VulkanCommandBuffer::EndRendering() {
		//This function does nothing because, due to dynamic rendering requirements, it cannot be executed in a secondary command buffer
		//However, this has to happen after all secondaries have been executed, and thus the code happens in Submit.
		//I know it's weird; sorry!
	}
}