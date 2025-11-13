#include "Cacao/GPU.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Window.hpp"
#include "VulkanModule.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

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

	GPUCommand VulkanModule::StartRenderingCmd(glm::vec3 clearColor) {
		return CommandWithFn([clearColor](CommandBuffer* cmd) {
			//Make sure this is a Vulkan buffer
			VulkanCommandBuffer* vkCmd = [&cmd]() -> VulkanCommandBuffer* {
				if(VulkanCommandBuffer* vkcb = dynamic_cast<VulkanCommandBuffer*>(cmd)) {
					return vkcb;
				} else {
					Check<BadTypeException>(false, "Cannot submit a non-Vulkan command buffer to the Vulkan backend!");
					throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
				}
			}();

			//Setup graphics
			vkCmd->imm.get().SetupGfx();
			GfxHandler& gfx = vkCmd->imm.get().gfx.value();

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
			gfx.MakeDrawable(vkCmd);

			//Configure output region
			vkCmd->vk().setViewport(0, viewport);
			vkCmd->vk().setScissor(0, scissor);

			//Begin rendering (this will clear the screen due to values set above)
			vkCmd->vk().beginRendering(renderInfo);
		});
	}

	GPUCommand VulkanModule::EndRenderingCmd() {
		return CommandWithFn([](CommandBuffer* cmd) {
			//Make sure this is a Vulkan buffer
			VulkanCommandBuffer* vkCmd = [&cmd]() -> VulkanCommandBuffer* {
				if(VulkanCommandBuffer* vkcb = dynamic_cast<VulkanCommandBuffer*>(cmd)) {
					return vkcb;
				} else {
					Check<BadTypeException>(false, "Cannot submit a non-Vulkan command buffer to the Vulkan backend!");
					throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
				}
			}();

			//Obtain graphics handler
			GfxHandler& gfx = vkCmd->imm.get().gfx.value();

			//End rendering
			vkCmd->vk().endRendering();

			//Make image presentable
			gfx.MakePresentable(vkCmd);
		});
	}

	GPUCommand VulkanModule::PresentCmd() {
		//Presenting is implicit with Vulkan if Immediate::SetupGfx has been ccalled
		//This is because you can't call queue.present before submitting the command buffer
		return CommandWithFn([](CommandBuffer*) {});
	}
}