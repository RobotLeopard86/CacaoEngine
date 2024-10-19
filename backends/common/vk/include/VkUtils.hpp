#pragma once

#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.hpp"

#include <queue>
#include <mutex>

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

	inline bool didGenShaders = false;
}