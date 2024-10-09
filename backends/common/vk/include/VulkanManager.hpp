#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.hpp"

#include "VkFrame.hpp"

#include <vector>

namespace Cacao {
	class VulkanManager {
	  public:
		//Get the instance or create one if it doesn't exist
		static VulkanManager* GetInstance();

		vk::Instance instance;
		vk::PhysicalDevice physDev;
		vk::Device dev;
		vk::Queue graphicsQueue;
		vk::SurfaceKHR surface;
		vk::SwapchainKHR swapchain;
		std::vector<vk::Image> images;
		std::vector<vk::ImageView> imageViews;
		std::vector<VkFrame> frames;
		vk::CommandPool renderPool, immediatePool;
		vma::Allocator allocator;

	  private:
		static VulkanManager* instance;
		static bool instanceExists;
	};
}