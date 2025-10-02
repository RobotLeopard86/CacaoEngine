#include "VulkanMesh.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/GPU.hpp"
#include "VulkanModule.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace Cacao {
	void VulkanMeshImpl::Realize(bool& success) {
		//Allocate vertex data buffers
		vk::BufferCreateInfo vertexCI({}, sizeof(Vertex) * vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, 0);
		vma::AllocationCreateInfo vertexAllocCI(vma::AllocationCreateFlagBits::eWithinBudget, vma::MemoryUsage::eAutoPreferDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vk::BufferCreateInfo vertexUpCI({}, sizeof(Vertex) * vertices.size(), vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, 0);
		vma::AllocationCreateInfo vertexUpAllocCI(vma::AllocationCreateFlagBits::eWithinBudget | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite, vma::MemoryUsage::eAuto,
			vk::MemoryPropertyFlagBits::eHostVisible);
		Allocated<vk::Buffer> vboUp;
		try {
			vbo = vulkan->allocator.createBuffer(vertexCI, vertexAllocCI);
			vboUp = vulkan->allocator.createBuffer(vertexUpCI, vertexUpAllocCI);
		} catch(const vk::SystemError& vkse) {
			std::stringstream msg;
			msg << "Encountered Vulkan exception during vertex buffer creation: " << vkse.what();
			Check<ExternalException>(false, msg.str());
		}

		//Unpack index buffer data from vec3 to uints
		std::vector<unsigned int> ibd(indices.size() * 3);
		for(std::size_t i = 0; i < indices.size(); i++) {
			glm::vec3 idx = indices[i];
			ibd[i * 3] = idx.x;
			ibd[(i * 3) + 1] = idx.y;
			ibd[(i * 3) + 2] = idx.z;
		}

		//Allocate index data buffers
		vk::BufferCreateInfo indexCI({}, sizeof(unsigned int) * ibd.size(), vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, 0);
		vma::AllocationCreateInfo indexAllocCI(vma::AllocationCreateFlagBits::eWithinBudget, vma::MemoryUsage::eAutoPreferDevice, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vk::BufferCreateInfo indexUpCI({}, sizeof(unsigned int) * ibd.size(), vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, 0);
		vma::AllocationCreateInfo indexUpAllocCI(vma::AllocationCreateFlagBits::eWithinBudget | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite, vma::MemoryUsage::eAuto,
			vk::MemoryPropertyFlagBits::eHostVisible);
		Allocated<vk::Buffer> iboUp;
		try {
			ibo = vulkan->allocator.createBuffer(indexCI, indexAllocCI);
			iboUp = vulkan->allocator.createBuffer(indexUpCI, indexUpAllocCI);
		} catch(const vk::SystemError& vkse) {
			std::stringstream msg;
			msg << "Encountered Vulkan exception during index buffer creation: " << vkse.what();
			Check<ExternalException>(false, msg.str());
		}

		//Copy data data to upload buffers
		void* gpuMem;
		Check<ExternalException>(vulkan->allocator.mapMemory(vboUp.alloc, &gpuMem) == vk::Result::eSuccess, "Failed to map vertex upload buffer memory!");
		std::memcpy(gpuMem, vertices.data(), sizeof(Vertex) * vertices.size());
		vulkan->allocator.unmapMemory(vboUp.alloc);
		Check<ExternalException>(vulkan->allocator.mapMemory(iboUp.alloc, &gpuMem) == vk::Result::eSuccess, "Failed to map index upload buffer memory!");
		std::memcpy(gpuMem, ibd.data(), sizeof(unsigned int) * ibd.size());
		vulkan->allocator.unmapMemory(iboUp.alloc);

		//Transfer data from upload buffers to real buffers
		VulkanCommandBuffer vcb;
		{
			vk::BufferCopy2 copy(0UL, 0UL, sizeof(Vertex) * vertices.size());
			vk::CopyBufferInfo2 copyInfo(vboUp.obj, vbo.obj, copy);
			vcb->copyBuffer2(copyInfo);
		}
		{
			vk::BufferCopy2 copy(0UL, 0UL, sizeof(unsigned int) * ibd.size());
			vk::CopyBufferInfo2 copyInfo(iboUp.obj, ibo.obj, copy);
			vcb->copyBuffer2(copyInfo);
		}
		GPUManager::Get().Submit(std::move(vcb)).get();

		//Destroy upload buffers
		vulkan->allocator.destroyBuffer(vboUp.obj, vboUp.alloc);
		vulkan->allocator.destroyBuffer(iboUp.obj, iboUp.alloc);

		success = true;
	}

	void VulkanMeshImpl::DropRealized() {
		vulkan->allocator.destroyBuffer(vbo.obj, vbo.alloc);
		vulkan->allocator.destroyBuffer(ibo.obj, ibo.alloc);
	}

	Mesh::Impl* VulkanModule::ConfigureMesh() {
		return new VulkanMeshImpl();
	}
}