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
	  public:
		vk::CommandPool pool;
		vk::CommandBuffer cmd;
		vk::Fence fence;

		static Immediate Get();
		static void Cleanup();

	  private:
		static std::map<std::thread::id, Immediate> immediates;
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
	inline vma::Allocator allocator;
	inline Allocated<vk::Buffer> globalsUBO;
	inline void* globalsMem;
	inline Allocated<vk::Image> depthImage;
	inline vk::ImageView depthView;
	inline vk::Format selectedDF;
	constexpr inline glm::mat4 projectionCorrection(
		{1.0f, 0.0f, 0.0f, 0.0f}, //No X change
		{0.0f, -1.0f, 0.0f, 0.0f},//Invert Y
		{0.0f, 0.0f, 0.5f, 0.0f}, //Halve depth range: [-1, 1] (OpenGL range) -> [-0.5, 0.5]
		{0.0f, 0.0f, 0.5f, 1.0f});//Shift depth range: [-0.5, 0.5] => [0, 1] (Vulkan range)
}