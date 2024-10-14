#pragma once

#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.hpp"

#include "VkFrame.hpp"
#include "VkUtils.hpp"

#include <vector>
#include <thread>
#include <map>

namespace Cacao {
	struct Immediate {
		vk::CommandBuffer cmd;
		vk::Fence fence;
	};
	inline vk::Instance vk_instance;
	inline vk::PhysicalDevice physDev;
	inline vk::Device dev;
	inline vk::Queue graphicsQueue, immediateQueue;
	inline vk::SurfaceKHR surface;
	inline vk::SurfaceFormatKHR surfaceFormat;
	inline vk::SwapchainKHR swapchain;
	inline std::vector<vk::Image> images;
	inline std::vector<vk::ImageView> imageViews;
	inline std::vector<VkFrame> frames;
	inline vk::CommandPool renderPool, immediatePool;
	inline std::map<std::thread::id, Immediate> immediates;
	inline vma::Allocator allocator;
	inline Allocated<vk::Buffer> globalsUBO;
	inline void* globalsMem;
	inline Allocated<vk::Image> depthImage;
	inline vk::ImageView depthView;
	inline vk::Format selectedDF;
}