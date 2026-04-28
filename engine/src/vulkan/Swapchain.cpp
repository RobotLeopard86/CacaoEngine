#include "VulkanModule.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Exceptions.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <cstdint>

namespace Cacao {
	void VulkanGPU::GenSwapchain() {
		//Lock the command buffer queue mutex
		//This will block the GPU thread from running more commands until we're done (that would be bad)
		std::lock_guard lk(vulkan->queueMtx);

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
		swapchainCI.setClipped(VK_TRUE);
		if(vulkan->swapchain.chain) swapchainCI.setOldSwapchain(vulkan->swapchain.chain);
		try {
			vk::SwapchainKHR newSwapchain = vulkan->dev.createSwapchainKHR(swapchainCI);
			if(vulkan->swapchain.chain) {
				for(vk::ImageView& view : vulkan->swapchain.views) {
					vulkan->dev.destroyImageView(view);
				}
				for(ViewImage& vi : vulkan->swapchain.depthImages) {
					vulkan->dev.destroyImageView(vi.view);
					vulkan->allocator.destroyImage(vi.obj, vi.alloc);
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

		//Create new depth objects
		vulkan->swapchain.depthImages = std::vector<ViewImage>(vulkan->swapchain.images.size());
		for(std::size_t i = 0; i < vulkan->swapchain.depthImages.size(); ++i) {
			//Get slot reference
			ViewImage& vi = vulkan->swapchain.depthImages[i];

			//Create new objects
			static vk::ImageCreateInfo depthCI({}, vk::ImageType::e2D, vulkan->selectedDF, {vulkan->swapchain.extent.width, vulkan->swapchain.extent.height, 1}, 1, 1, vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive);
			static vma::AllocationCreateInfo depthAllocCI({}, vma::MemoryUsage::eGpuOnly, vk::MemoryPropertyFlagBits::eDeviceLocal);
			auto [img, alloc] = vulkan->allocator.createImage(depthCI, depthAllocCI);
			try {
				vi.alloc = alloc;
				vi.obj = img;
			} catch(vk::SystemError& err) {
				Check<ExternalException>(false, "Failed to create new depth image!", [i]() {
					for(std::size_t j = i - 1; j >= 0; --j) {
						ViewImage& badVI = vulkan->swapchain.depthImages[i];
						vulkan->dev.destroyImageView(badVI.view);
						vulkan->allocator.destroyImage(badVI.obj, badVI.alloc);
					}
				});
			}
			try {
				vk::ImageViewCreateInfo depthViewCI({}, vi.obj, vk::ImageViewType::e2D, vulkan->selectedDF, {},
					vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
				vi.view = vulkan->dev.createImageView(depthViewCI);
			} catch(vk::SystemError& err) {
				Check<ExternalException>(false, "Failed to create new depth image view!", [i]() {
					for(std::size_t j = i; j >= 0; --j) {
						ViewImage& badVI = vulkan->swapchain.depthImages[i];
						vulkan->dev.destroyImageView(badVI.view);
						vulkan->allocator.destroyImage(badVI.obj, badVI.alloc);
					}
				});
			}
		}

		//Destroy old contexts
		for(std::unique_ptr<RenderCommandContext>& rcc : vulkan->swapchain.renderContexts) {
			if(rcc->sync.semaphore) vulkan->dev.destroySemaphore(rcc->sync.semaphore);
		}
		for(std::unique_ptr<ImageContext>& ic : vulkan->swapchain.imageContexts) {
			if(ic->acquire) vulkan->dev.destroySemaphore(ic->acquire);
			if(ic->render) vulkan->dev.destroySemaphore(ic->render);
		}
		vulkan->swapchain.renderContexts.clear();
		vulkan->swapchain.imageContexts.clear();

		//Create and setup new contexts
		for(unsigned int i = 0; i < vulkan->swapchain.images.size(); ++i) {
			//Render context
			std::unique_ptr<RenderCommandContext> rcc = std::make_unique<RenderCommandContext>();
			rcc->imageIndex = UINT32_MAX;
			vk::SemaphoreTypeCreateInfoKHR semTypeCI(vk::SemaphoreType::eTimeline, 0);
			try {
				rcc->sync.semaphore = vulkan->dev.createSemaphore(vk::SemaphoreCreateInfo {{}, &semTypeCI});
				rcc->sync.doneValue = 0;
			} catch(vk::SystemError& err) {
				Check<ExternalException>(false, "Failed to create synchronization objects for rendering command context!");
			}
			vulkan->swapchain.renderContexts.push_back(std::move(rcc));

			//Image context
			std::unique_ptr<ImageContext> ic = std::make_unique<ImageContext>();
			vk::SemaphoreCreateInfo semCI {};
			try {
				ic->acquire = vulkan->dev.createSemaphore(semCI);
				ic->render = vulkan->dev.createSemaphore(semCI);
			} catch(vk::SystemError& err) {
				Check<ExternalException>(false, "Failed to create synchronization objects for image context!");
			}
			vulkan->swapchain.imageContexts.push_back(std::move(ic));
		}
	}
}