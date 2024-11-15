#pragma once

#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.hpp"
#include "glm/vec4.hpp"

#include "VkFrame.hpp"
#include "VkUtils.hpp"

#include <vector>
#include <thread>
#include <map>
#include <mutex>

namespace Cacao {
	struct Immediate {
		vk::CommandPool pool;
		vk::CommandBuffer cmd;
		vk::Fence fence;
	};
	inline vk::Instance vk_instance;
	inline vk::PhysicalDevice physDev;
	inline vk::Device dev;
	inline vk::Queue queue;
	inline std::mutex queueMtx;
	inline vk::SurfaceKHR surface;
	inline vk::SurfaceFormatKHR surfaceFormat;
	inline vk::SwapchainKHR swapchain;
	inline std::vector<vk::Image> images;
	inline std::vector<vk::ImageView> imageViews;
	inline std::vector<VkFrame> frames;
	inline vk::CommandPool renderPool;
	inline std::map<std::thread::id, Immediate> immediates;
	inline vma::Allocator allocator;
	inline Allocated<vk::Buffer> globalsUBO;
	inline void* globalsMem;
	inline Allocated<vk::Image> depthImage;
	inline vk::ImageView depthView;
	inline vk::Format selectedDF;
	constexpr inline glm::mat4 projectionCorrection(
		{1.0f, 0.0f, 0.0f, 0.0f}, //No X change
		{0.0f, -1.0f, 0.0f, 0.0f},//Flip Y
		{0.0f, 0.0f, 0.5f, 0.5f}, //Adjust depth range (OpenGL [-1, 1] -> Vulkan [0, 1])
		{0.0f, 0.0f, 0.0f, 1.0f});//No W change
}