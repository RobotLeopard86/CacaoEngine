#include "Cacao/Window.hpp"
#include "VulkanModule.hpp"

namespace Cacao {
	void GfxHandler::MakeDrawable(VulkanCommandBuffer* cmd) {
		//Make our image drawable
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 0, 0, vulkan->swapchain.images[imageIdx],
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			cmd->vk().pipelineBarrier2(transition);
		}

		//Make the depth image drawable
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, 0, 0, vulkan->depth.obj,
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			cmd->vk().pipelineBarrier2(transition);
		}
	}

	void GfxHandler::MakePresentable(VulkanCommandBuffer* cmd) {
		//Make our image presentable
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, 0, 0, vulkan->swapchain.images[imageIdx],
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			cmd->vk().pipelineBarrier2(transition);
		}

		//Put the depth image into a read-only format to not leave it in a rendering state
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eDepthAttachmentOptimal, vk::ImageLayout::eDepthReadOnlyOptimal, 0, 0, vulkan->depth.obj,
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			cmd->vk().pipelineBarrier2(transition);
		}
	}

	void VulkanCommandBuffer::StartRendering(glm::vec3 clearColor) {
		//Setup graphics
		if(!imm.get().gfx) imm.get().SetupGfx();
		GfxHandler& gfx = *(imm.get().gfx);

		//Calculate window extent
		auto caSize = Window::Get().GetContentAreaSize();
		vk::Extent2D extent(caSize.x, caSize.y);
		auto surfc = vulkan->physDev.getSurfaceCapabilitiesKHR(vulkan->surface);
		extent.width = std::clamp(extent.width, surfc.minImageExtent.width, surfc.maxImageExtent.width);
		extent.height = std::clamp(extent.height, surfc.minImageExtent.height, surfc.maxImageExtent.height);

		//Setup rendering info
		vk::Viewport viewport(0.0f, 0.0f, float(extent.width), float(extent.height), 0.0f, 1.0f);
		vk::Rect2D scissor({0, 0}, extent);
		vk::RenderingAttachmentInfo colorAttachment(vulkan->swapchain.views[gfx.imageIdx], vk::ImageLayout::eColorAttachmentOptimal, {}, {}, {},
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearColorValue(std::array<float, 4> {clearColor.r, clearColor.g, clearColor.b, 1.0f}));
		vk::RenderingAttachmentInfo depthAttachment(vulkan->depth.view, vk::ImageLayout::eDepthAttachmentOptimal, {}, {}, {},
			vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ClearDepthStencilValue(1.0f, 0.0f));
		vk::RenderingInfo renderInfo({}, vk::Rect2D({0, 0}, extent), 1, 0, colorAttachment, &depthAttachment);

		//Make image drawable
		gfx.MakeDrawable(this);

		//Configure output region
		vk().setViewport(0, viewport);
		vk().setScissor(0, scissor);

		//Begin rendering (this will clear the screen due to values set above)
		vk().beginRendering(renderInfo);
	}

	void VulkanCommandBuffer::EndRendering() {
		//Obtain graphics handler
		GfxHandler& gfx = *(imm.get().gfx);

		//End rendering
		vk().endRendering();

		//Make image presentable
		gfx.MakePresentable(this);
	}
}