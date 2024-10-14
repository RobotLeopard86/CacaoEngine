#pragma once

#include "vulkan/vulkan.hpp"

#include "VkUtils.hpp"
#include "VulkanCoreObjects.hpp"

namespace Cacao {
	struct Skybox::SkyboxData {
		Allocated<vk::Buffer> vertexBuffer;
		bool vbufReady;

		~SkyboxData() {
			if(vbufReady) {
				allocator.destroyBuffer(vertexBuffer.obj, vertexBuffer.alloc);
				vbufReady = false;
			}
		}
	};
}