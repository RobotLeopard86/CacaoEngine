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
		}
	}

	UIView::~UIView() {
		//Destroy buffer objects
		dev.destroyImageView(frontBuffer->view);
		allocator.destroyImage(frontBuffer->tex.obj, frontBuffer->tex.alloc);
		dev.destroyImageView(backBuffer->view);
		allocator.destroyImage(backBuffer->tex.obj, backBuffer->tex.alloc);
	}

	void UIView::Bind(int slot) {
		CheckException(hasRendered, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot bind unrendered UI view!")
		CheckException(!bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot bind bound UI view!")
		CheckException(activeShader, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot bind UI view when there is no bound shader!")
		CheckException(std::find_if(activeShader->imageSlots.begin(), activeShader->imageSlots.end(), [slot](auto is) { return is.second.binding == slot; }) != activeShader->imageSlots.end(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Requested texture slot does not exist in bound shader!")

		//Create update info
		vk::DescriptorImageInfo dii(VK_NULL_HANDLE, frontBuffer->view, vk::ImageLayout::eShaderReadOnlyOptimal);
		vk::WriteDescriptorSet wds(VK_NULL_HANDLE, slot, 0, vk::DescriptorType::eCombinedImageSampler, dii);

		//Update descriptor set
		activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);

		currentSlot = slot;
		bound = true;
	}

	void UIView::Unbind() {
		CheckException(bound, Exception::GetExceptionCodeFromMeaning("BadBindState"), "Cannot unbind unbound UI view!")
		CheckException(activeShader, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot unbind UI view when there is no bound shader!")

		//Create update info
		vk::DescriptorImageInfo dii(VK_NULL_HANDLE, VK_NULL_HANDLE, vk::ImageLayout::eUndefined);
		vk::WriteDescriptorSet wds(VK_NULL_HANDLE, currentSlot, 0, vk::DescriptorType::eCombinedImageSampler, dii);

		//Update descriptor set
		activeFrame->cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, activeShader->pipelineLayout, 0, wds);

		currentSlot = -1;
		bound = false;
	}

	void UIView::Draw(std::map<unsigned short, std::vector<std::shared_ptr<UIRenderable>>> renderables) {
		//Delete old back buffer
		dev.destroyImageView(backBuffer->view);
		allocator.destroyImage(backBuffer->tex.obj, backBuffer->tex.alloc);

		//Regenerate back buffer
		vk::ImageCreateInfo bufImageCI({}, vk::ImageType::e2D, surfaceFormat.format, {size.x, size.y, 1}, 1, 1, vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo bufAllocCI({}, vma::MemoryUsage::eGpuOnly, vk::MemoryPropertyFlagBits::eDeviceLocal);
		auto [img, alloc] = allocator.createImage(bufImageCI, bufAllocCI);
		backBuffer->tex = {.alloc = alloc, .obj = img};
		vk::ImageViewCreateInfo imageViewCI(
			{}, backBuffer->tex.obj, vk::ImageViewType::e2D, surfaceFormat.format, {},
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
		backBuffer->view = dev.createImageView(imageViewCI);

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
		vk::Extent2D extent((unsigned int)size.x, (unsigned int)size.y);
		{
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
		imm.cmd.setDepthCompareOp(vk::CompareOp::eAlways);

		//Transition image to drawable state
		{
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 0, 0, backBuffer->tex.obj, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
			vk::DependencyInfo cdDI({}, {}, {}, barrier);
			imm.cmd.pipelineBarrier2(cdDI);
		}

		//Set allocated object list and command buffer
		uiCmd = &(imm.cmd);
		allocatedObjects.clear();

		//Preprocess text renderables
		trCharInfos.clear();
		for(auto layer : renderables) {
			for(auto renderable : layer.second) {
				//If the cast fails, this wasn't a text renderable so we don't care
				try {
					Text::Renderable* tr = std::dynamic_pointer_cast<Text::Renderable>(renderable).get();
					if(tr) PreprocessTextRenderable(tr, size);
				} catch(...) {}
			}
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
			vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, backBuffer->tex.obj, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
			vk::DependencyInfo cdDI({}, {}, {}, barrier);
			imm.cmd.pipelineBarrier2(cdDI);
		}

		//End recording
		imm.cmd.end();

		//Wait for and reset fence just in case
		if(dev.getFenceStatus(imm.fence) == vk::Result::eSuccess) {
			vk::Result fenceWait = dev.waitForFences(imm.fence, VK_TRUE, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1000)).count());
			CheckException(fenceWait == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("WaitExpired"), "Waited too long for immediate fence reset!")
			dev.resetFences(imm.fence);
		}

		//Submit command buffer and wait
		vk::CommandBufferSubmitInfo cbsi(imm.cmd);
		vk::SubmitInfo2 si({}, {}, cbsi);
		imm.queue.submit2(si, imm.fence);
		dev.waitForFences(imm.fence, VK_TRUE, INFINITY);

		//Clean up allocated objects
		for(auto obj : allocatedObjects) {
			switch(obj.index()) {
				case 0: {
					auto image = std::get<Allocated<vk::Image>>(obj);
					allocator.destroyImage(image.obj, image.alloc);
					break;
				}
				case 1: {
					auto buffer = std::get<Allocated<vk::Buffer>>(obj);
					allocator.destroyBuffer(buffer.obj, buffer.alloc);
					break;
				}
				case 2: {
					auto iview = std::get<vk::ImageView>(obj);
					dev.destroyImageView(iview);
					break;
				}
			}
		}
		allocatedObjects.clear();

		//Restore the real frame object
		VkFrame* fake = activeFrame;
		activeFrame = realActive;
		delete fake;
	}
}