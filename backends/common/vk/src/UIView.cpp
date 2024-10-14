#include "UI/UIView.hpp"

#include "VkUIView.hpp"
#include "VkUtils.hpp"
#include "ActiveItems.hpp"
#include "VulkanCoreObjects.hpp"
#include "Graphics/Window.hpp"
#include "UIViewShaderManager.hpp"

namespace Cacao {
	//Required initialization of static members
	Shader* UIView::shader = nullptr;
	vk::DescriptorSet* UIView::Buffer::boundDS = nullptr;

	UIView::UIView()
	  : size(0), bound(false), currentSlot(-1), hasRendered(false) {
		//Create buffers
		frontBuffer.reset(new Buffer());
		backBuffer.reset(new Buffer());

		//Create buffer objects
		std::vector<std::shared_ptr<Buffer>> bufs = {this->frontBuffer, this->backBuffer};
		vk::ImageCreateInfo bufImageCI({}, vk::ImageType::e2D, surfaceFormat.format, {1, 1, 1}, 1, 1, vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo bufAllocCI({}, vma::MemoryUsage::eGpuOnly, vk::MemoryPropertyFlagBits::eDeviceLocal);
		for(auto buf : bufs) {
			//Create image
			auto [img, alloc] = allocator.createImage(bufImageCI, bufAllocCI);
			buf->tex = {.alloc = alloc, .obj = img};

			//Create image view
			vk::ImageViewCreateInfo imageViewCI(
				{}, buf->tex.obj, vk::ImageViewType::e2D, surfaceFormat.format, {},
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
			buf->view = dev.createImageView(imageViewCI);

			//Create sampler
			vk::SamplerCreateInfo samplerCI({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge,
				vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0.0f, VK_FALSE, 0.0f, VK_FALSE, vk::CompareOp::eNever, 0.0f,
				VK_REMAINING_MIP_LEVELS, vk::BorderColor::eIntTransparentBlack, VK_FALSE);
			buf->sampler = dev.createSampler(samplerCI);
		}
	}

	UIView::~UIView() {
		//Destroy buffer objects
		dev.destroySampler(frontBuffer->sampler);
		dev.destroyImageView(frontBuffer->view);
		allocator.destroyImage(frontBuffer->tex.obj, frontBuffer->tex.alloc);
		dev.destroySampler(backBuffer->sampler);
		dev.destroyImageView(backBuffer->view);
		allocator.destroyImage(backBuffer->tex.obj, backBuffer->tex.alloc);
	}

	void UIView::Bind(int slot) {
		CheckException(hasRendered, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind unrendered UI view!")
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound UI view!")
		CheckException(activeShader, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot bind UI view when there is no bound shader!")
		CheckException(std::find_if(activeShader->imageSlots.begin(), activeShader->imageSlots.end(), [slot](auto is) { return is.second == slot; }) != activeShader->imageSlots.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Requested texture slot does not exist in bound shader!")

		//Create update info
		vk::DescriptorImageInfo dii(frontBuffer->sampler, frontBuffer->view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet wds(activeShader->dset, slot, 0, vk::DescriptorType::eCombinedImageSampler, dii);

		//Update descriptor set
		dev.updateDescriptorSets(wds, {});

		currentSlot = slot;
		Buffer::boundDS = &activeShader->dset;
		bound = true;
	}

	void UIView::Unbind() {
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound UI view!")

		//Check if bound descriptor set still exists
		if(!Buffer::boundDS || (Buffer::boundDS && *(Buffer::boundDS))) {
			bound = false;
			return;
		}

		//Create update info
		vk::DescriptorImageInfo dii(VK_NULL_HANDLE, VK_NULL_HANDLE, vk::ImageLayout::eUndefined);
		vk::WriteDescriptorSet wds(*(Buffer::boundDS), currentSlot, 0, vk::DescriptorType::eCombinedImageSampler, dii);

		//Update descriptor set
		dev.updateDescriptorSets(wds, {});

		currentSlot = -1;
		bound = false;
		Buffer::boundDS = nullptr;
	}

	void UIView::Draw(std::map<unsigned short, std::vector<std::shared_ptr<UIRenderable>>> renderables) {
		//Regenerate back buffer
		vk::ImageCreateInfo bufImageCI({}, vk::ImageType::e2D, surfaceFormat.format, {1, 1, 1}, 1, 1, vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo bufAllocCI({}, vma::MemoryUsage::eGpuOnly, vk::MemoryPropertyFlagBits::eDeviceLocal);
		auto [img, alloc] = allocator.createImage(bufImageCI, bufAllocCI);
		backBuffer->tex = {.alloc = alloc, .obj = img};
		vk::ImageViewCreateInfo imageViewCI(
			{}, backBuffer->tex.obj, vk::ImageViewType::e2D, surfaceFormat.format, {},
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
		backBuffer->view = dev.createImageView(imageViewCI);
		vk::SamplerCreateInfo samplerCI({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge,
			vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0.0f, VK_FALSE, 0.0f, VK_FALSE, vk::CompareOp::eNever, 0.0f,
			VK_REMAINING_MIP_LEVELS, vk::BorderColor::eIntTransparentBlack, VK_FALSE);
		backBuffer->sampler = dev.createSampler(samplerCI);

		//Fetch immediate
		Immediate imm = immediates[std::this_thread::get_id()];

		//Create projection matrix
		glm::mat4 project = projectionCorrection * glm::ortho(0.0f, float(size.x), 0.0f, float(size.y));

		//Create a fake frame object
		VkFrame* realActive = activeFrame;
		activeFrame = new VkFrame();
		activeFrame->cmd = imm.cmd;
		activeFrame->fence = imm.fence;

		//Reset command buffer
		vk::CommandBufferBeginInfo begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		imm.cmd.reset({});
		imm.cmd.begin(begin);

		//Calculate extent
		vk::Extent2D extent;
		{
			glm::ivec2 winSize = Window::GetInstance()->GetContentAreaSize();
			auto surfc = physDev.getSurfaceCapabilitiesKHR(surface);
			extent.width = std::clamp(extent.width, surfc.minImageExtent.width, surfc.maxImageExtent.width);
			extent.height = std::clamp(extent.height, surfc.minImageExtent.height, surfc.maxImageExtent.height);
		}

		//Set dynamic state
		vk::Viewport viewport(0.0f, 0.0f, float(extent.width), float(extent.height), 0.0f, 1.0f);
		vk::Rect2D scissor({0, 0}, extent);
		imm.cmd.setViewport(0, viewport);
		imm.cmd.setScissor(0, scissor);
		imm.cmd.setColorBlendEnableEXT(0, VK_TRUE);
		vk::ColorBlendEquationEXT equ(vk::BlendFactor::eOne, vk::BlendFactor::eOneMinusSrcColor, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eOneMinusSrcAlpha);
		imm.cmd.setColorBlendEquationEXT(0, equ);
		imm.cmd.setDepthTestEnable(VK_FALSE);

		//Transition image to drawable state
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 0, 0, backBuffer->tex.obj,
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			imm.cmd.pipelineBarrier2(transition);
		}

		//Start rendering
		vk::RenderingAttachmentInfo colorAttachment(backBuffer->view, vk::ImageLayout::eColorAttachmentOptimal, {}, {}, {}, vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore, vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f));
		vk::RenderingInfo renderingInfo({}, vk::Rect2D({0, 0}, extent), 1, 0, colorAttachment);
		imm.cmd.beginRendering(renderingInfo);

		//Render each layer
		int furthest = 0;
		for(const auto& kv : renderables) {
			if(kv.first > furthest) furthest = kv.first;
		}
		for(int i = furthest; i >= 0; i--) {
			const std::vector<std::shared_ptr<UIRenderable>>& layer = renderables[i];
			for(auto renderable : layer) {
				renderable->Draw(size, project);
			}
		}

		//End rendering
		imm.cmd.endRendering();

		//Transition image to sampleable state
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
				vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, backBuffer->tex.obj,
				vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
			vk::DependencyInfo transition({}, {}, {}, barrier);
			imm.cmd.pipelineBarrier2(transition);
		}

		//End recording
		imm.cmd.end();

		//Restore the real frame object
		VkFrame* fake = activeFrame;
		activeFrame = realActive;
		delete fake;
	}
}