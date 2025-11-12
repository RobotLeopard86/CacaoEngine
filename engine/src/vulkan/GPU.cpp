#include "Cacao/GPU.hpp"
#include "VulkanModule.hpp"
#include "Cacao/Exceptions.hpp"

#include <atomic>
#include <future>
#include <mutex>
#include <optional>
#include <thread>

namespace Cacao {
	std::map<std::thread::id, Immediate> Immediate::immediates = {};
	std::vector<GfxHandler> GfxHandler::handlers = {};

	Immediate& Immediate::Get() {
		//Check if we have an immediate for this thread already
		auto threadID = std::this_thread::get_id();
		if(immediates.contains(threadID)) {
			Immediate& ret = immediates.at(threadID);
			ret.cmd.reset();
			return ret;
		}

		//Make a new immediate
		const auto& [it, success] = immediates.emplace();
		Check<MiscException>(success, "Failed to create immediate!");
		Immediate& imm = it->second;
		try {
			vk::CommandPoolCreateInfo ipoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, 0);
			imm.pool = vulkan->dev.createCommandPool(ipoolCI);
			vk::CommandBufferAllocateInfo allocCI(imm.pool, vk::CommandBufferLevel::ePrimary, 1);
			imm.cmd = vulkan->dev.allocateCommandBuffers(allocCI)[0];
			imm.fence = vulkan->dev.createFence({vk::FenceCreateFlagBits::eSignaled});
		} catch(...) {
			Check<ExternalException>(false, "Failed to setup immediate object for thread!", [&imm]() {
				vulkan->dev.freeCommandBuffers(imm.pool, imm.cmd);
				vulkan->dev.destroyCommandPool(imm.pool);
				vulkan->dev.destroyFence(imm.fence);
			});
		}

		return imm;
	}

	void Immediate::Cleanup() {
		//Wait for the device to be idle
		vulkan->dev.waitIdle();

		for(auto& imm : immediates) {
			vulkan->dev.freeCommandBuffers(imm.second.pool, imm.second.cmd);
			vulkan->dev.destroyCommandPool(imm.second.pool);
			vulkan->dev.destroyFence(imm.second.fence);
		}
		immediates.clear();
	}

	void Immediate::SetupGfx() {
		while(!gfx.has_value()) {
			//Iterate over all the handlers until we find one that isn't in use
			for(GfxHandler& handler : GfxHandler::handlers) {
				//The exchange method returns the previous value of the atomic
				//So if it returns false, we know this handler was free and we have now reserved it
				if(!handler.inUse.exchange(true, std::memory_order_acq_rel)) {
					gfx = std::move(handler);
					return;
				}
			}
			std::this_thread::yield();
		}
	}

	GPUManager::Impl* VulkanModule::ConfigureGPUManager() {
		return new VulkanGPU();
	}

	VulkanCommandBuffer::VulkanCommandBuffer()
	  : imm(Immediate::Get()), lock(imm.get().mtx), promise() {
		//Start command buffer recording
		vk::CommandBufferBeginInfo cbbi(vk::CommandBufferUsageFlagBits::eOneTimeSubmit | vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		imm.get().cmd.begin(cbbi);
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other)
	  : imm(std::move(other.imm)), lock(std::move(other.lock)), promise(std::move(other.promise)) {}

	VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other) {
		imm = std::move(other.imm);
		lock = std::move(other.lock);
		promise = std::move(other.promise);
		return *this;
	}

	void VulkanCommandBuffer::Execute() {
		//End recording
		imm.get().cmd.end();

		//Build submission info
		vk::CommandBufferSubmitInfo cbsi(imm.get().cmd);
		vk::SubmitInfo2 submit({}, {}, cbsi);

		//Submit buffer to queue
		{
			std::lock_guard lk(vulkan->queueMtx);
			vulkan->queue.submit2(submit, imm.get().fence);
		}
	}

	std::shared_future<void> VulkanGPU::SubmitCmdBuffer(std::unique_ptr<CommandBuffer>&& cmd) {
		//Make sure this is a Vulkan buffer
		std::unique_ptr<VulkanCommandBuffer> vkCmd = [&cmd]() -> std::unique_ptr<VulkanCommandBuffer> {
			if(VulkanCommandBuffer* vcb = dynamic_cast<VulkanCommandBuffer*>(cmd.release())) {
				return std::unique_ptr<VulkanCommandBuffer>(vcb);
			} else {
				Check<BadTypeException>(false, "Cannot submit a non-Vulkan command buffer to the Vulkan backend!");
				throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
			}
		}();

		//Submit underlying commands
		vkCmd->Execute();

		//Get a future
		std::shared_future<void> fut = vkCmd->promise.get_future().share();

		//Move buffer into submission list for result tracking
		{
			std::lock_guard lk(mutex);
			submitted.push_back(std::move(vkCmd));
		}

		//Return future
		return fut;
	}

	void VulkanGPU::RunloopIteration() {
		//Go through all the submitted command buffers and see if they're done
		std::lock_guard lk(mutex);
		for(auto it = submitted.begin(); it != submitted.end();) {
			std::unique_ptr<VulkanCommandBuffer>& vcb = *it;
			if(vulkan->dev.getFenceStatus(vcb->imm.get().fence) == vk::Result::eSuccess) {
				//Reset the fence
				vulkan->dev.resetFences(vcb->imm.get().fence);

				//Release graphics handler if needed
				vcb->imm.get().gfx->inUse.store(false);
				vcb->imm.get().gfx = std::nullopt;

				//Set the promise
				vcb->promise.set_value();

				//Erase the object and update the iterator
				it = submitted.erase(it);
			} else {
				//Increment like normal
				++it;
			}
		}
	}
}