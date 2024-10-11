#pragma once

#include "vulkan/vulkan.hpp"

#include "VkUtils.hpp"

namespace Cacao {
	//Struct for data required for a Vulkan mesh
	struct Mesh::MeshData {
		Allocated<vk::Buffer> vertexBuffer, indexBuffer;
	};
}