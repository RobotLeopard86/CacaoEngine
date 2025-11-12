#include "VulkanModule.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Exceptions.hpp"

#include <atomic>
#include <cstdint>

namespace Cacao {
	void GenSwapchain() {
		//Lock the command buffer queue mutex
		//This will block the GPU thread from running more commands until we're done (that would be bad)
		std::lock_guard lk(vulkan->queueMtx);

		//Now claim all the graphics handlers so nobody can mess with them while we're working since we may need to touch them
		unsigned int obtained = 0;
		while(obtained < GfxHandler::handlers.size()) {
			for(GfxHandler& handler : GfxHandler::handlers) {
				//The exchange method returns the previous value of the atomic
				//So if it returns false, we know this handler was free and we have now reserved it
				if(!handler.inUse.exchange(true, std::memory_order_acq_rel)) {
					++obtained;
				}
			}
		}

		//Wait for device to be idle
		vulkan->dev.waitIdle();

		//Delete the old swapchain images if they existed
		if(vulkan->swapchain.chain) {
			vulkan->dev.destroySwapchainKHR(vulkan->swapchain.chain);
			vulkan->dev.destroyImageView(vulkan->depth.view);
			vulkan->allocator.destroyImage(vulkan->depth.obj, vulkan->depth.alloc);
			for(auto iv : vulkan->swapchain.views) {
				vulkan->dev.destroyImageView(iv);
			}
		}

		//Get surface capabilities
		auto surfc = vulkan->physDev.getSurfaceCapabilitiesKHR(vulkan->surface);

		//Calculate extent
		auto caSize = Window::Get().GetContentAreaSize();
		vk::Extent2D extent(caSize.x, caSize.y);
		extent.width = std::clamp(extent.width, surfc.minImageExtent.width, surfc.maxImageExtent.width);
		extent.height = std::clamp(extent.height, surfc.minImageExtent.height, surfc.maxImageExtent.height);

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
			vulkan->surfaceFormat.format, vulkan->surfaceFormat.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, 0,
			vk::SurfaceTransformFlagBitsKHR::eInherit, vk::CompositeAlphaFlagBitsKHR::eInherit, presentMode);
		try {
			vulkan->swapchain.chain = vulkan->dev.createSwapchainKHR(swapchainCI);
		} catch(vk::SystemError& err) {
			Check<ExternalException>(false, "Failed to create swapchain!");
		}

		//Get new swapchain images
		vulkan->swapchain.images = vulkan->dev.getSwapchainImagesKHR(vulkan->swapchain.chain);

		//Create new swapchain image views
		std::vector<vk::ImageView> swapchainImageViews(vulkan->swapchain.images.size());
		for(std::size_t i = 0; i < vulkan->swapchain.images.size(); ++i) {
			vk::ImageViewCreateInfo imageViewCI(
				{}, vulkan->swapchain.images[i], vk::ImageViewType::e2D, vulkan->surfaceFormat.format, {},
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
			try {
				swapchainImageViews[i] = vulkan->dev.createImageView(imageViewCI);
			} catch(vk::SystemError& err) {
				Check<ExternalException>(false, "Failed to create swapchain image view!", [&swapchainImageViews, &i]() {
					for(; i > 0; --i) {
						vulkan->dev.destroyImageView(swapchainImageViews[i]);
					}
					vulkan->dev.destroyImageView(swapchainImageViews[0]);
				});
			}
		}
		vulkan->swapchain.views = swapchainImageViews;

		//Create new depth image and view
		vk::ImageCreateInfo depthCI({}, vk::ImageType::e2D, vulkan->selectedDF, {extent.width, extent.height, 1}, 1, 1, vk::SampleCountFlagBits::e1,
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
			Check<ExternalException>(false, "Failed to create depth image view!", [&swapchainImageViews]() {
				for(std::size_t i = 0; i < swapchainImageViews.size(); ++i) {
					vulkan->dev.destroyImageView(swapchainImageViews[i]);
				}
			});
		}

		//Update graphics handlers
		if(GfxHandler::handlers.size() > vulkan->swapchain.views.size()) {
			//Too many handlers, remove the extras
			unsigned int diff = GfxHandler::handlers.size() - vulkan->swapchain.views.size();
			auto it = GfxHandler::handlers.end();
			while(std::distance(it, GfxHandler::handlers.end()) < diff) --it;
			GfxHandler::handlers.erase(it, GfxHandler::handlers.end());
		} else if(GfxHandler::handlers.size() < vulkan->swapchain.views.size()) {
			//Too few, add some
			unsigned int diff = vulkan->swapchain.views.size() - GfxHandler::handlers.size();
			for(unsigned int i = 0; i < diff; ++i) {
				//Setup handler object
				GfxHandler handler;
				handler.imageIdx = UINT32_MAX;
				handler.inUse.store(true);
				try {
					handler.imageFence = vulkan->dev.createFence({vk::FenceCreateFlagBits::eSignaled});
					handler.acquireImage = vulkan->dev.createSemaphore({});
					handler.doneRendering = vulkan->dev.createSemaphore({});
				} catch(vk::SystemError& err) {
					Check<ExternalException>(false, "Failed to setup Vulkan graphics handler object!", [&swapchainImageViews]() {
						vulkan->dev.destroyImageView(vulkan->depth.view);
						vulkan->allocator.destroyImage(vulkan->depth.obj, vulkan->depth.alloc);
						for(std::size_t i = 0; i < swapchainImageViews.size(); ++i) {
							vulkan->dev.destroyImageView(swapchainImageViews[i]);
						}
					});
				}

				//Add to map
				GfxHandler::handlers.push_back(std::move(handler));
			}
		}

		//Release handlers
		//Queue lock will be released automatically by stack pop
		for(GfxHandler& handler : GfxHandler::handlers) {
			handler.inUse.store(false, std::memory_order_acq_rel);
		}
	}
}