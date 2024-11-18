#include "3D/Mesh.hpp"
#include "VkMeshData.hpp"
#include "Core/Log.hpp"
#include "Core/Exception.hpp"
#include "Core/Engine.hpp"
#include "VulkanCoreObjects.hpp"
#include "ActiveItems.hpp"

#include <future>

namespace Cacao {
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices)
	  : Asset(false), vertices(vertices), indices(indices) {
		//Create native data
		nativeData.reset(new MeshData());
	}

	std::shared_future<void> Mesh::CompileAsync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled mesh!");
		return Engine::GetInstance()->GetThreadPool()->enqueue([this]() { this->CompileSync(); }).share();
	}

	void Mesh::CompileSync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled mesh!");
		//Unpack index buffer data from vec3 to floats
		std::vector<unsigned int> ibd(indices.size() * 3);
		for(int i = 0; i < indices.size(); i++) {
			glm::vec3 idx = indices[i];
			ibd[i * 3] = idx.x;
			ibd[(i * 3) + 1] = idx.y;
			ibd[(i * 3) + 2] = idx.z;
		}

		//Make buffer create infos
		auto vbsz = sizeof(Vertex) * vertices.size();
		auto ibsz = sizeof(unsigned int) * ibd.size();
		vk::BufferCreateInfo vertexCI({}, vbsz, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer);
		vk::BufferCreateInfo vertexUpCI({}, vbsz, vk::BufferUsageFlagBits::eTransferSrc);
		vk::BufferCreateInfo indexCI({}, ibsz, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer);
		vk::BufferCreateInfo indexUpCI({}, ibsz, vk::BufferUsageFlagBits::eTransferSrc);
		vma::AllocationCreateInfo uploadAllocCI({}, vma::MemoryUsage::eCpuToGpu);
		vma::AllocationCreateInfo bufferAllocCI({}, vma::MemoryUsage::eGpuOnly);

		//Create buffer objects
		Allocated<vk::Buffer> vertex = {}, index = {}, vertexUp = {}, indexUp = {};
		{
			auto [buffer, alloc] = allocator.createBuffer(vertexCI, bufferAllocCI);
			vertex.alloc = alloc;
			vertex.obj = buffer;
		}
		{
			auto [buffer, alloc] = allocator.createBuffer(indexCI, bufferAllocCI);
			index.alloc = alloc;
			index.obj = buffer;
		}
		{
			auto [buffer, alloc] = allocator.createBuffer(vertexUpCI, uploadAllocCI);
			vertexUp.alloc = alloc;
			vertexUp.obj = buffer;
		}
		{
			auto [buffer, alloc] = allocator.createBuffer(indexUpCI, uploadAllocCI);
			indexUp.alloc = alloc;
			indexUp.obj = buffer;
		}

		//Upload data to the GPU
		void* gpuMem;
		allocator.mapMemory(vertexUp.alloc, &gpuMem);
		std::memcpy(gpuMem, vertices.data(), vbsz);
		allocator.unmapMemory(vertexUp.alloc);
		allocator.mapMemory(indexUp.alloc, &gpuMem);
		std::memcpy(gpuMem, indices.data(), ibsz);
		allocator.unmapMemory(indexUp.alloc);

		//Record a resource copy from the upload buffers to the real buffers
		Immediate imm = immediates.at(std::this_thread::get_id());
		vk::CommandBufferBeginInfo copyBegin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		imm.cmd.begin(copyBegin);
		{
			vk::BufferCopy2 copy(0UL, 0UL, vbsz);
			vk::CopyBufferInfo2 copyInfo(vertexUp.obj, vertex.obj, copy);
			imm.cmd.copyBuffer2(copyInfo);
		}
		{
			vk::BufferCopy2 copy(0UL, 0UL, ibsz);
			vk::CopyBufferInfo2 copyInfo(indexUp.obj, index.obj, copy);
			imm.cmd.copyBuffer2(copyInfo);
		}
		imm.cmd.end();

		//Wait for and reset fence just in case
		if(dev.getFenceStatus(imm.fence) == vk::Result::eSuccess) {
			vk::Result fenceWait = dev.waitForFences(imm.fence, VK_TRUE, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1000)).count());
			CheckException(fenceWait == vk::Result::eSuccess, Exception::GetExceptionCodeFromMeaning("WaitExpired"), "Waited too long for immediate fence reset!");
			dev.resetFences(imm.fence);
		}

		//Submit and wait
		vk::CommandBufferSubmitInfo cbsi(imm.cmd);
		vk::SubmitInfo2 si({}, {}, cbsi);
		SubmitCommandBuffer(si, imm.fence);
		dev.waitForFences(imm.fence, VK_TRUE, INFINITY);

		//Destroy upload buffers
		allocator.destroyBuffer(vertexUp.obj, vertexUp.alloc);
		allocator.destroyBuffer(indexUp.obj, indexUp.alloc);

		//Set values in native data
		nativeData->vertexBuffer = vertex;
		nativeData->indexBuffer = index;

		compiled = true;
	}

	void Mesh::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled mesh!");

		//Destroy buffers
		allocator.destroyBuffer(nativeData->vertexBuffer.obj, nativeData->vertexBuffer.alloc);
		allocator.destroyBuffer(nativeData->indexBuffer.obj, nativeData->indexBuffer.alloc);

		compiled = false;
	}

	void Mesh::Draw() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot draw uncompiled mesh!");
		CheckException(activeFrame, Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot draw mesh when there is no active frame object!");
		VkFrame f = *activeFrame;

		//Bind vertex and index buffer
		constexpr std::array<vk::DeviceSize, 1> offsets = {{0}};
		f.cmd.bindVertexBuffers(0, nativeData->vertexBuffer.obj, offsets);
		f.cmd.bindIndexBuffer(nativeData->indexBuffer.obj, offsets[0], vk::IndexType::eUint32);

		//Draw mesh
		f.cmd.drawIndexed(indices.size() * 3, 1, 0, 0, 0);
	}
}