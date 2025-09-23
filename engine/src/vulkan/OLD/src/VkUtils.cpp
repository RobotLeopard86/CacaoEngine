#include "VkUtils.hpp"

#include "VulkanCoreObjects.hpp"
#include "Graphics/Window.hpp"
#include "UI/Shaders.hpp"
#include "VkShader.hpp"
#include "UIViewShaderManager.hpp"

namespace Cacao {
	void GenFrameObjects() {
		//If there was no size change, return
		if(imageViews.size() == frames.size()) return;

		//Cache old objects and clear frame vector
		auto reusables = frames;
		frames = std::vector<VkFrame>(imageViews.size());

		if(imageViews.size() < reusables.size()) {
			//Reuse as many frames as needed
			unsigned int i = 0;
			for(; i < frames.size(); i++) {
				frames[i] = reusables[i];
			}

			//Delete old objects
			std::vector<vk::CommandBuffer> oldBuffers;
			for(; i < reusables.size(); i++) {
				oldBuffers.push_back(reusables[i].cmd);
				dev.destroyFence(reusables[i].fence);
				dev.destroySemaphore(reusables[i].acquireSemaphore);
				dev.destroySemaphore(reusables[i].renderSemaphore);
			}
			dev.freeCommandBuffers(renderPool, oldBuffers);
		} else {
			//Reuse all frames
			unsigned int i = 0;
			for(; i < reusables.size(); i++) {
				frames[i] = reusables[i];
			}

			//Create new frame objects
			vk::CommandBufferAllocateInfo allocInfo(renderPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(frames.size() - reusables.size()));
			std::vector<vk::CommandBuffer> cbufs;
			try {
				cbufs = dev.allocateCommandBuffers(allocInfo);
			} catch(vk::SystemError& err) {
				std::rethrow_exception(std::current_exception());
			}
			for(; i < frames.size(); i++) {
				VkFrame f = {};
				f.cmd = cbufs[i - reusables.size()];
				try {
					f.acquireSemaphore = dev.createSemaphore({});
				} catch(vk::SystemError& err) {
					std::rethrow_exception(std::current_exception());
				}
				try {
					f.renderSemaphore = dev.createSemaphore({});
				} catch(vk::SystemError& err) {
					std::rethrow_exception(std::current_exception());
				}
				try {
					f.fence = dev.createFence({vk::FenceCreateFlagBits::eSignaled});
				} catch(vk::SystemError& err) {
					std::rethrow_exception(std::current_exception());
				}
				frames[i] = f;
			}
		}
	}

	void GenSwapchain() {
		//Wait for device to be idle
		dev.waitIdle();

		//Delete the old swapchain if it exists
		if(swapchain) {
			dev.destroySwapchainKHR(swapchain);
			for(auto iv : imageViews) {
				dev.destroyImageView(iv);
			}
			dev.destroyImageView(depthView);
			allocator.destroyImage(depthImage.obj, depthImage.alloc);
		}

		//Get surface capabilities
		auto surfc = physDev.getSurfaceCapabilitiesKHR(surface);

		//Calculate extent
		auto caSize = Window::Get()->GetContentAreaSize();
		vk::Extent2D extent(caSize.x, caSize.y);
		extent.width = std::clamp(extent.width, surfc.minImageExtent.width, surfc.maxImageExtent.width);
		extent.height = std::clamp(extent.height, surfc.minImageExtent.height, surfc.maxImageExtent.height);

		//Check for present mode
		auto pmodes = physDev.getSurfacePresentModesKHR(surface);
		CheckException(std::find(pmodes.cbegin(), pmodes.cend(), presentMode) != pmodes.cend(), Exception::GetExceptionCodeFromMeaning("NonexistentValue"), "The requested present mode is not available!");

		//Make new swapchain
		vk::SwapchainCreateInfoKHR swapchainCI(
			{}, surface, std::clamp((surfc.minImageCount + 1), surfc.minImageCount, (surfc.maxImageCount > 0 ? surfc.maxImageCount : UINT32_MAX)),
			surfaceFormat.format, surfaceFormat.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment);
		try {
			swapchain = dev.createSwapchainKHR(swapchainCI);
		} catch(vk::SystemError& err) {
			std::rethrow_exception(std::current_exception());
		}

		//Get new swapchain images
		images = dev.getSwapchainImagesKHR(swapchain);

		//Create new swapchain image views
		std::vector<vk::ImageView> swapchainImageViews(images.size());
		for(std::size_t i = 0; i < images.size(); ++i) {
			vk::ImageViewCreateInfo imageViewCI(
				{}, images[i], vk::ImageViewType::e2D, surfaceFormat.format, {},
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
			swapchainImageViews[i] = dev.createImageView(imageViewCI);
		}
		imageViews = swapchainImageViews;

		//Create new depth image and view
		vk::ImageCreateInfo depthCI({}, vk::ImageType::e2D, selectedDF, {extent.width, extent.height, 1}, 1, 1, vk::SampleCountFlagBits::e1,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo depthAllocCI({}, vma::MemoryUsage::eGpuOnly, vk::MemoryPropertyFlagBits::eDeviceLocal);
		{
			auto [img, alloc] = allocator.createImage(depthCI, depthAllocCI);
			depthImage = {.alloc = alloc, .obj = img};
		}
		vk::ImageViewCreateInfo depthViewCI(
			{}, depthImage.obj, vk::ImageViewType::e2D, selectedDF, {},
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
		try {
			depthView = dev.createImageView(depthViewCI);
		} catch(vk::SystemError& err) {
			std::rethrow_exception(std::current_exception());
		}

		if(!didGenShaders) {
			//Generate UI shaders (they need the surface fornat)
			//Compile UI view shader
			uivsm.Compile();
			while(!uivsm->IsCompiled()) {
				std::this_thread::sleep_for(std::chrono::microseconds(1));
			}

			//Generate other UI shaders
			GenShaders();
			while(!TextShaders::shader->IsCompiled() || !ImageShaders::shader->IsCompiled()) {
				std::this_thread::sleep_for(std::chrono::microseconds(1));
			}

			didGenShaders = true;
		}

		//Regenerate frame objects
		GenFrameObjects();
	}

	std::map<std::thread::id, Immediate> Immediate::immediates = std::map<std::thread::id, Immediate>();
}