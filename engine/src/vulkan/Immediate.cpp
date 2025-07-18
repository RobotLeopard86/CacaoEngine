#include "Module.hpp"
#include "Cacao/Exceptions.hpp"

namespace Cacao {
	std::map<std::thread::id, Immediate> Immediate::immediates = {};

	Immediate Immediate::Get() {
		//Check if we have an immediate for this thread already
		auto threadID = std::this_thread::get_id();
		if(immediates.contains(threadID)) {
			Immediate ret = immediates.at(threadID);
			ret.cmd.reset();
			return ret;
		}

		//Make a new immediate
		Immediate imm;
		try {
			vk::CommandPoolCreateInfo ipoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, 0);
			imm.pool = vulkan->dev.createCommandPool(ipoolCI);
			vk::CommandBufferAllocateInfo allocCI(imm.pool, vk::CommandBufferLevel::ePrimary, 1);
			imm.cmd = vulkan->dev.allocateCommandBuffers(allocCI)[0];
			imm.fence = vulkan->dev.createFence({vk::FenceCreateFlagBits::eSignaled});
			immediates.insert_or_assign(threadID, imm);
		} catch(...) {
			Check<ExternalException>(false, "Failed to create immediate object for thread!");
		}
		return imm;
	}

	void Immediate::Cleanup() {
		//Wait for the device to be idle
		vulkan->dev.waitIdle();

		for(auto imm : immediates) {
			vulkan->dev.freeCommandBuffers(imm.second.pool, imm.second.cmd);
			vulkan->dev.destroyCommandPool(imm.second.pool);
			vulkan->dev.destroyFence(imm.second.fence);
		}
		immediates.clear();
	}

	void Immediate::Submit(bool wait) {
		//Build submission info
		vk::CommandBufferSubmitInfo cbsi(cmd);
		vk::SubmitInfo2 submit({}, {}, cbsi);

		//Submit buffer to queue
		{
			std::lock_guard lk(vulkan->immqLock);
			vulkan->immQueue.submit2(submit, fence);
		}

		//If requested, wait
		if(wait) vulkan->dev.waitForFences(fence, VK_TRUE, UINT64_MAX);
	}
}