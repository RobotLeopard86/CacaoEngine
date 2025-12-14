#include "VulkanModule.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Exceptions.hpp"

#include <atomic>
#include <cstdint>

namespace Cacao {
	void SetupRenderingContext(std::unique_ptr<RenderCommandContext>& rcc) {
		rcc->imageIndex = UINT32_MAX;
		rcc->available.store(true);
		vk::SemaphoreCreateInfo semCreate {};
		vk::SemaphoreTypeCreateInfoKHR semTypeCI(vk::SemaphoreType::eTimeline, 0);
		try {
			rcc->fence = vulkan->dev.createFence({});
			rcc->acquire = vulkan->dev.createSemaphore(semCreate);
			rcc->render = vulkan->dev.createSemaphore(semCreate);
			rcc->sync.semaphore = vulkan->dev.createSemaphore(vk::SemaphoreCreateInfo {{}, &semTypeCI});
			rcc->sync.doneValue = 0;
		} catch(vk::SystemError& err) {
			if(rcc->fence) vulkan->dev.destroyFence(rcc->fence);
			if(rcc->acquire) vulkan->dev.destroySemaphore(rcc->acquire);
			if(rcc->render) vulkan->dev.destroySemaphore(rcc->render);
			Check<ExternalException>(false, "Failed to create synchronization objects for rendering command context!");
		}
	}

	void GenSwapchain() {
		//Set regen flags
		vulkan->swapchain.regenInProgress.store(true, std::memory_order_seq_cst);
		vulkan->swapchain.regenRequested.store(false, std::memory_order_seq_cst);

		//Lock the command buffer queue mutex
		//This will block the GPU thread from running more commands until we're done (that would be bad)
		std::lock_guard lk(vulkan->queueMtx);

		//Wait for device to be idle
		vulkan->dev.waitIdle();

		//Get surface capabilities
		vk::SurfaceCapabilitiesKHR surfc = vulkan->physDev.getSurfaceCapabilitiesKHR(vulkan->surface);

		//Calculate extent
		glm::uvec2 caSize = Window::Get().GetContentAreaSize();
		vulkan->swapchain.extent = vk::Extent2D {caSize.x, caSize.y};
		vulkan->swapchain.extent.width = std::clamp(vulkan->swapchain.extent.width, surfc.minImageExtent.width, surfc.maxImageExtent.width);
		vulkan->swapchain.extent.height = std::clamp(vulkan->swapchain.extent.height, surfc.minImageExtent.height, surfc.maxImageExtent.height);

		//Decide present mode
		auto pmodes = vulkan->physDev.getSurfacePresentModesKHR(vulkan->surface);
		vk::PresentModeKHR presentMode;
		if(vulkan->vsync) {
			presentMode = vk::PresentModeKHR::eMailbox;
			if(std::find(pmodes.cbegin(), pmodes.cend(), presentMode) == pmodes.cend()) {
				presentMode = vk::PresentModeKHR::eFifo;
				Check<NonexistentValueException>(std::find(pmodes.cbegin(), pmodes.cend(), presentMode) != pmodes.cend(), "The requested present mode is not available!");
			}
		} else {
			presentMode = vk::PresentModeKHR::eImmediate;
			Check<NonexistentValueException>(std::find(pmodes.cbegin(), pmodes.cend(), presentMode) != pmodes.cend(), "The requested present mode is not available!");
		}

		//Make new swapchain
		vk::SwapchainCreateInfoKHR swapchainCI(
			{}, vulkan->surface, std::clamp((surfc.minImageCount + 1), surfc.minImageCount, (surfc.maxImageCount > 0 ? surfc.maxImageCount : UINT32_MAX)),
			vulkan->surfaceFormat.format, vulkan->surfaceFormat.colorSpace, vulkan->swapchain.extent, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive);
		swapchainCI.setPresentMode(presentMode);
		if(vulkan->swapchain.chain) swapchainCI.setOldSwapchain(vulkan->swapchain.chain);
		try {
			vk::SwapchainKHR newSwapchain = vulkan->dev.createSwapchainKHR(swapchainCI);
			if(vulkan->swapchain.chain) {
				for(vk::ImageView& view : vulkan->swapchain.views) {
					vulkan->dev.destroyImageView(view);
				}
				vulkan->dev.destroySwapchainKHR(vulkan->swapchain.chain);
			}
			vulkan->swapchain.views.clear();
			vulkan->swapchain.chain = newSwapchain;
		} catch(vk::SystemError& err) {
			Check<ExternalException>(false, "Failed to create swapchain!");
		}

		//Get new swapchain images
		vulkan->swapchain.images = vulkan->dev.getSwapchainImagesKHR(vulkan->swapchain.chain);

		//Create new swapchain image views
		vulkan->swapchain.views = std::vector<vk::ImageView>(vulkan->swapchain.images.size());
		for(std::size_t i = 0; i < vulkan->swapchain.images.size(); ++i) {
			vk::ImageViewCreateInfo imageViewCI(
				{}, vulkan->swapchain.images[i], vk::ImageViewType::e2D, vulkan->surfaceFormat.format, {},
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
			try {
				vulkan->swapchain.views[i] = vulkan->dev.createImageView(imageViewCI);
			} catch(vk::SystemError& err) {
				Check<ExternalException>(false, "Failed to create swapchain image view!", [&i]() {
					for(; i >= 0; --i) {
						vulkan->dev.destroyImageView(vulkan->swapchain.views[i]);
					}
					vulkan->swapchain.views.clear();
				});
			}
		}

		//Destroy old depth objects
		vulkan->dev.destroyImageView(vulkan->depth.view);
		vulkan->allocator.destroyImage(vulkan->depth.obj, vulkan->depth.alloc);

		//Create new depth image and view
		vk::ImageCreateInfo depthCI({}, vk::ImageType::e2D, vulkan->selectedDF, {vulkan->swapchain.extent.width, vulkan->swapchain.extent.height, 1}, 1, 1, vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo depthAllocCI({}, vma::MemoryUsage::eGpuOnly, vk::MemoryPropertyFlagBits::eDeviceLocal);
		{
			auto [img, alloc] = vulkan->allocator.createImage(depthCI, depthAllocCI);
			vulkan->depth.alloc = alloc;
			vulkan->depth.obj = img;
		}
		vk::ImageViewCreateInfo depthViewCI(
			{}, vulkan->depth.obj, vk::ImageViewType::e2D, vulkan->selectedDF, {},
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
		try {
			vulkan->depth.view = vulkan->dev.createImageView(depthViewCI);
		} catch(vk::SystemError& err) {
			Check<ExternalException>(false, "Failed to create depth image view!", []() {
				for(std::size_t i = 0; i < vulkan->swapchain.views.size(); ++i) {
					vulkan->dev.destroyImageView(vulkan->swapchain.views[i]);
				}
				vulkan->swapchain.views.clear();
			});
		}

		//Setup render contexts (number of contexts always needs to match number of image views)
		std::size_t numContexts = vulkan->swapchain.renderContexts.size();
		std::size_t numImages = vulkan->swapchain.images.size();
		if(numContexts != numImages) {
			if(numImages > numContexts) {
				//Create, setup, and add new contexts
				for(unsigned int i = 0; i < (numImages - numContexts); ++i) {
					std::unique_ptr<RenderCommandContext> newCtx = std::make_unique<RenderCommandContext>();
					SetupRenderingContext(newCtx);
					vulkan->swapchain.renderContexts.push_back(std::move(newCtx));
				}
			} else {
				//We have too many contexts, so we'll reuse what we can and delete the rest by RAII
				std::vector<std::unique_ptr<RenderCommandContext>> oldContexts = std::exchange(vulkan->swapchain.renderContexts, {});
				vulkan->swapchain.renderContexts.resize(numImages);
				unsigned int i = 0;
				for(; i < numImages; ++i) {
					vulkan->swapchain.renderContexts[i] = std::move(oldContexts[i]);
					vulkan->swapchain.renderContexts[i]->imageIndex = UINT32_MAX;
					vulkan->swapchain.renderContexts[i]->available.store(true);
				}
				for(; i < numContexts; ++i) {
					std::unique_ptr<RenderCommandContext>& rcc = oldContexts[i];
					if(rcc->fence) vulkan->dev.destroyFence(rcc->fence);
					if(rcc->acquire) vulkan->dev.destroySemaphore(rcc->acquire);
					if(rcc->render) vulkan->dev.destroySemaphore(rcc->render);
					if(rcc->sync.semaphore) vulkan->dev.destroySemaphore(rcc->sync.semaphore);
				}
			}
		}

		//Regen done
		vulkan->swapchain.regenInProgress.store(false);
	}
}