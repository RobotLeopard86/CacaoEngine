#include "VulkanModule.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/Exceptions.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"

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

		//Create new acquire semaphores
		for(vk::Semaphore& sem : vulkan->swapchain.acquireSems) {
			vulkan->dev.destroySemaphore(sem);
		}
		vulkan->swapchain.acquireSems.resize(vulkan->swapchain.images.size());
		for(vk::Semaphore& sem : vulkan->swapchain.acquireSems) {
			sem = vulkan->dev.createSemaphore(vk::SemaphoreCreateInfo {});
		}
		vulkan->swapchain.imageSemIndices.assign(vulkan->swapchain.acquireSems.size(), UINT16_MAX);

		//Create new depth objects
		vulkan->swapchain.depthImages = std::vector<ViewImage>(vulkan->swapchain.images.size());
		for(std::size_t i = 0; i < vulkan->swapchain.depthImages.size(); ++i) {
			//Get slot reference
			ViewImage& vi = vulkan->swapchain.depthImages[i];

			//Create new objects
			vk::ImageCreateInfo depthCI({}, vk::ImageType::e2D, vulkan->selectedDF, {vulkan->swapchain.extent.width, vulkan->swapchain.extent.height, 1}, 1, 1, vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive);
			vma::AllocationCreateInfo depthAllocCI({}, vma::MemoryUsage::eGpuOnly, vk::MemoryPropertyFlagBits::eDeviceLocal);
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

		//Re-create descriptor pool
		if(vulkan->descriptorPool) {
			vulkan->dev.resetDescriptorPool(vulkan->descriptorPool);
			vulkan->dev.destroyDescriptorPool(vulkan->descriptorPool);
		}
		vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, vulkan->swapchain.images.size());
		vk::DescriptorPoolCreateInfo poolCI({}, vulkan->swapchain.images.size(), poolSize);
		try {
			vulkan->descriptorPool = vulkan->dev.createDescriptorPool(poolCI);
		} catch(vk::SystemError& err) {
			Check<ExternalException>(false, "Failed to create new descriptor pool!");
		}

		//Destroy old contexts
		for(RenderCommandContext& rc : vulkan->swapchain.contexts) {
			if(rc.rendered) vulkan->dev.destroySemaphore(rc.rendered);
			if(rc.inFlight) vulkan->dev.destroyFence(rc.inFlight);
			if(rc.sync.semaphore) vulkan->dev.destroySemaphore(rc.sync.semaphore);
			if(rc.globals.obj) {
				vulkan->allocator.unmapMemory(rc.globals.alloc);
				vulkan->allocator.destroyBuffer(rc.globals.obj, rc.globals.alloc);
			}
		}
		vulkan->swapchain.contexts.resize(vulkan->swapchain.images.size());

		//Create and setup new contexts
		vk::SemaphoreTypeCreateInfo semTypeCI(vk::SemaphoreType::eTimeline);
		vk::SemaphoreCreateInfo semaphoreCI {};
		vk::SemaphoreCreateInfo timelineCI({}, &semTypeCI);
		vk::FenceCreateInfo fenceCI(vk::FenceCreateFlagBits::eSignaled);
		vk::BufferCreateInfo globalsCI({}, sizeof(GlobalsData), vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
		vma::AllocationCreateInfo uboAllocCI({}, vma::MemoryUsage::eCpuToGpu);
		vk::DescriptorSetAllocateInfo setAI(vulkan->descriptorPool, vulkan->engineSetLayout);
		for(unsigned int i = 0; i < vulkan->swapchain.images.size(); ++i) {
			RenderCommandContext& rc = vulkan->swapchain.contexts[i];
			try {
				rc.rendered = vulkan->dev.createSemaphore(semaphoreCI);
				rc.inFlight = vulkan->dev.createFence(fenceCI);
				rc.sync.semaphore = vulkan->dev.createSemaphore(timelineCI);
				rc.sync.doneValue = 0;
				rc.globals = vulkan->allocator.createBuffer(globalsCI, uboAllocCI);
				rc.set = vulkan->dev.allocateDescriptorSets(setAI)[0];
			} catch(vk::SystemError& err) {
				Check<ExternalException>(false, "Failed to create objects for rendering context!");
			}
			try {
				vk::DescriptorBufferInfo globalsDBI(rc.globals.obj, 0, vk::WholeSize);
				vk::WriteDescriptorSet write(rc.set, 0, 0, 1, vk::DescriptorType::eUniformBuffer, VK_NULL_HANDLE, &globalsDBI);
				vulkan->dev.updateDescriptorSets(write, {});
			} catch(vk::SystemError& err) {
				Check<ExternalException>(false, "Failed to bind rendering context descriptor set objects!");
			}
			Check<ExternalException>(vulkan->allocator.mapMemory(rc.globals.alloc, &rc.globals.mem) == vk::Result::eSuccess, "Failed to map rendering context UBO memory!");
		}
	}
}