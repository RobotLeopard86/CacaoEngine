#pragma once

#include "vk_mem_alloc.hpp"
#include "vulkan/vulkan.hpp"

#include <queue>
#include <mutex>
#include <future>

namespace Cacao {
	void GenSwapchain();
	void GenFrameObjects();

	inline vk::PresentModeKHR presentMode;

	template<typename T>
	struct Allocated {
	  public:
		vma::Allocation alloc;
		T obj;
	};

	//Utility for submitting command buffers
	void SubmitCommandBuffer(vk::SubmitInfo2 submitInfo, vk::Fence fence);

	inline bool didGenShaders = false;

	//"Null" image (set active when a texture is unbound)
	inline Allocated<vk::Image> nullImage;
	inline vk::ImageView nullView;
}