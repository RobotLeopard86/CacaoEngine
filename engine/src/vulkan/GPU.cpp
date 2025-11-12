#include "Cacao/GPU.hpp"
#include "VulkanModule.hpp"
#include "Cacao/Exceptions.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <atomic>
#include <future>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>

namespace Cacao {
	std::set<Immediate*> Immediate::imms = {};
	std::vector<GfxHandler> GfxHandler::handlers = {};

	Immediate& Immediate::Get() {
		static thread_local Immediate imm = []() {
			Immediate imm;
			if(!imm.ready) {
				try {
					vk::CommandPoolCreateInfo ipoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, 0);
					imm.pool = vulkan->dev.createCommandPool(ipoolCI);
					vk::CommandBufferAllocateInfo allocCI(imm.pool, vk::CommandBufferLevel::ePrimary, 1);
					imm.cmd = vulkan->dev.allocateCommandBuffers(allocCI)[0];
					imm.fence = vulkan->dev.createFence({vk::FenceCreateFlagBits::eSignaled});
					imm.ready = true;
				} catch(...) {
					Check<ExternalException>(false, "Failed to setup immediate object for thread!", [&imm]() {
						vulkan->dev.freeCommandBuffers(imm.pool, imm.cmd);
						vulkan->dev.destroyCommandPool(imm.pool);
						vulkan->dev.destroyFence(imm.fence);
					});
				}
			}
			return imm;
		}();
		imms.insert(&imm);
		return imm;
	}

	void Immediate::Cleanup() {
		//Wait for the device to be idle
		vulkan->dev.waitIdle();

		for(Immediate* imm : imms) {
			vulkan->dev.freeCommandBuffers(imm->pool, imm->cmd);
			vulkan->dev.destroyCommandPool(imm->pool);
			vulkan->dev.destroyFence(imm->fence);
		}
		imms.clear();
	}

	void Immediate::SetupGfx() {
		while(!gfx.has_value()) {
			//Iterate over all the handlers until we find one that isn't in use
			for(GfxHandler& handler : GfxHandler::handlers) {
				//The exchange method returns the previous value of the atomic
				//So if it returns false, we know this handler was free and we have now reserved it
				if(!handler.inUse.exchange(true, std::memory_order_acq_rel)) {
					gfx = std::make_optional<std::reference_wrapper<GfxHandler>>(handler);
					gfx->get().Acquire();
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
	  : imm(Immediate::Get()), promise() {
		//Acquire immediate
		imm.get().accessMgr.acquire();

		//Start command buffer recording
		vk::CommandBufferBeginInfo cbbi(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		imm.get().cmd.begin(cbbi);
	}

	VulkanCommandBuffer::~VulkanCommandBuffer() {
		//Release immediate
		imm.get().accessMgr.release();
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other)
	  : imm(std::move(other.imm)), promise(std::move(other.promise)) {}

	VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other) {
		imm = std::move(other.imm);
		promise = std::move(other.promise);
		return *this;
	}

	void VulkanCommandBuffer::Execute() {
		//End recording
		imm.get().cmd.end();

		//Build submission info
		vk::SubmitInfo2 submit;
		vk::CommandBufferSubmitInfo cbsi(imm.get().cmd);
		if(imm.get().gfx.has_value()) {
			vk::SemaphoreSubmitInfo semWait(imm.get().gfx->get().acquireImage, 0, vk::PipelineStageFlagBits2::eAllCommands);
			vk::SemaphoreSubmitInfo semSignal(imm.get().gfx->get().doneRendering, 0, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
			submit = vk::SubmitInfo2({}, semWait, cbsi, semSignal);
		} else {
			submit = vk::SubmitInfo2({}, {}, cbsi);
		}

		//Reset fence if needed
		if(vulkan->dev.getFenceStatus(imm.get().fence) == vk::Result::eSuccess) {
			vk::Result fenceWait = vulkan->dev.waitForFences(imm.get().fence, VK_TRUE, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1000)).count());
			Check<ExternalException>(fenceWait == vk::Result::eSuccess, "Waited too long for immediate fence reset!");
			vulkan->dev.resetFences(imm.get().fence);
		}

		//Submit buffer to queue
		{
			std::lock_guard lk(vulkan->queueMtx);
			vulkan->queue.submit2(submit, imm.get().fence);
		}

		//Present if graphics is in use
		if(imm.get().gfx.has_value()) {
			vk::PresentInfoKHR presentInfo(imm.get().gfx->get().doneRendering, vulkan->swapchain.chain, imm.get().gfx->get().imageIdx);
			vulkan->queue.presentKHR(presentInfo);
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
				if(vcb->imm.get().gfx.has_value()) {
					vcb->imm.get().gfx->get().inUse.store(false, std::memory_order_release);
					vcb->imm.get().gfx = std::nullopt;
				}

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

	void GfxHandler::Acquire() {
		if(imageIdx != UINT32_MAX) return;

		//Try to acquire the next swapchain image
		try {
			//Reset fence
			vulkan->dev.waitForFences(imageFence, VK_TRUE, UINT64_MAX);
			vulkan->dev.resetFences(imageFence);

			//Acquire image
			vk::AcquireNextImageInfoKHR acquireInfo(vulkan->swapchain.chain, UINT64_MAX, acquireImage, imageFence);
			auto result = vulkan->dev.acquireNextImage2KHR(acquireInfo);
			if(result.result != vk::Result::eSuccess) throw vk::SystemError(result.result, "Failed to acquire swapchain image for unknown reason.");
			imageIdx = result.value;
		} catch(vk::SystemError& err) {
			//Is the swapchain out of date?
			//If so, we can regenerate and try again
			if(err.code() == vk::Result::eSuboptimalKHR || err.code() == vk::Result::eErrorOutOfDateKHR) {
				GenSwapchain();
				imageIdx = UINT32_MAX;
				Acquire();
				return;
			}

			//Other error, can't proceed
			std::stringstream msg;
			msg << "Failed to acquire swapchain image: " << err.what();
			Check<ExternalException>(false, msg.str());
		}

		//Wait for image acquisition to finish
		vulkan->dev.waitForFences(imageFence, VK_TRUE, UINT64_MAX);
		vulkan->dev.resetFences(imageFence);
	}
}