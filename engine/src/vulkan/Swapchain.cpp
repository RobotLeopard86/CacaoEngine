#include "VulkanModule.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Exceptions.hpp"
#include "ImplAccessor.hpp"
#include "impl/GPUManager.hpp"

#include <atomic>
#include <cstdint>

namespace Cacao {
	void SetupRenderingContext(std::unique_ptr<RenderCommandContext>& rcc) {
		rcc->imageIndex = UINT32_MAX;
		rcc->available.store(true);
		vk::SemaphoreCreateInfo semCreate {};
		vk::SemaphoreTypeCreateInfoKHR semTypeCI(vk::SemaphoreType::eTimeline, 0);
		try {
			rcc->acquire = vulkan->dev.createSemaphore(semCreate);
			rcc->render = vulkan->dev.createSemaphore(semCreate);
			rcc->sync.semaphore = vulkan->dev.createSemaphore(vk::SemaphoreCreateInfo {{}, &semTypeCI});
			rcc->sync.doneValue = 0;
		} catch(vk::SystemError& err) {
			if(rcc->acquire) vulkan->dev.destroySemaphore(rcc->acquire);
			if(rcc->render) vulkan->dev.destroySemaphore(rcc->render);
			Check<ExternalException>(false, "Failed to create synchronization objects for rendering command context!");
		}
	}

	void GenSwapchain() {
		//Set regen flags
		vulkan->swapchain.regenInProgress.store(true, std::memory_order_seq_cst);
		vulkan->swapchain.regenRequested.store(false, std::memory_order_seq_cst);

		//Lock the command buffer queue mutex and regen lock
		//This will block the GPU thread from running more commands until we're done (that would be bad)
		std::lock_guard lk(IMPL(GPUManager).regenLock);
		std::lock_guard lk2(vulkan->queueMtx);

		//Wait for device to be idle
		vulkan->dev.waitIdle();

		//Get surface capabilities
		vulkan->capabilities = vulkan->physDev.getSurfaceCapabilitiesKHR(vulkan->surface);

		//Calculate extent
		glm::uvec2 caSize = Window::Get().GetContentAreaSize();
		vk::Extent2D extent {caSize.x, caSize.y};
		extent.width = std::clamp(extent.width, vulkan->capabilities.minImageExtent.width, vulkan->capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, vulkan->capabilities.minImageExtent.height, vulkan->capabilities.maxImageExtent.height);

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
			{}, vulkan->surface, std::clamp((vulkan->capabilities.minImageCount + 2), vulkan->capabilities.minImageCount, (vulkan->capabilities.maxImageCount > 0 ? vulkan->capabilities.maxImageCount : UINT32_MAX)),
			vulkan->surfaceFormat.format, vulkan->surfaceFormat.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive);
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

		//Set extent now that swapchain is created
		vulkan->swapchain.extent = extent;
#ifdef HAS_WAYLAND
		CommitAfterRegen();
#endif

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
		if(vulkan->swapchain.renderContexts.size() != vulkan->swapchain.images.size()) {
			//Destroy old contexts
			for(std::unique_ptr<RenderCommandContext>& rcc : vulkan->swapchain.renderContexts) {
				if(rcc->acquire) vulkan->dev.destroySemaphore(rcc->acquire);
				if(rcc->render) vulkan->dev.destroySemaphore(rcc->render);
				if(rcc->sync.semaphore) vulkan->dev.destroySemaphore(rcc->sync.semaphore);
			}
			vulkan->swapchain.renderContexts.clear();

			//Create and setup new contexts
			for(unsigned int i = 0; i < vulkan->swapchain.images.size(); ++i) {
				std::unique_ptr<RenderCommandContext> newCtx = std::make_unique<RenderCommandContext>();
				SetupRenderingContext(newCtx);
				vulkan->swapchain.renderContexts.push_back(std::move(newCtx));
			}
		}

		//Regen done
		//Locks will be released by stack unwind
		vulkan->swapchain.regenInProgress.store(false);
	}
}