#include "Cacao/GPU.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Time.hpp"
#include "VulkanModule.hpp"

#include <atomic>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace Cacao {
	std::set<Immediate*> Immediate::imms = {};
	std::vector<std::unique_ptr<GfxHandler>> GfxHandler::handlers = {};

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
						if(imm.cmd) vulkan->dev.freeCommandBuffers(imm.pool, imm.cmd);
						if(imm.pool) vulkan->dev.destroyCommandPool(imm.pool);
						if(imm.fence) vulkan->dev.destroyFence(imm.fence);
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
		while(!gfx) {
			//Iterate over all the handlers until we find one that isn't in use
			for(std::unique_ptr<GfxHandler>& handler : GfxHandler::handlers) {
				//The exchange method returns the previous value of the atomic
				//So if it returns false, we know this handler was free and we have now reserved it
				if(!handler->inUse.exchange(true, std::memory_order_acq_rel)) {
					gfx = handler.get();
					gfx->own = "gfx";
					gfx->tid = ::syscall(SYS_gettid);
					gfx->imageIdx = UINT32_MAX;
					gfx->Acquire();
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
		if(imm.get().gfx) {
			vk::SemaphoreSubmitInfo semWait(imm.get().gfx->acquireImage, 0, vk::PipelineStageFlagBits2::eAllCommands);
			vk::SemaphoreSubmitInfo semSignal(imm.get().gfx->doneRendering, 0, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
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

		//Obtain queue lock
		std::unique_lock<std::timed_mutex> lk(vulkan->queueMtx, std::defer_lock_t {});
		if(imm.get().gfx) {
			//If we have graphics, try to lock for a bit at a time
			//This is to allow for swapchain regen
			while(!lk.try_lock_for(time::fmilliseconds(1))) {
				//Let's see if we're trying to regenerate the swapchain
				if(vulkan->swapchain.regenAck.load(std::memory_order_relaxed)) {
					//Swapchain regen cannot take place because we're blocked
					//So we'll have to give up this frame
					//Unfortuanately, that happens sometimes
					imm.get().gfx->imageIdx = UINT32_MAX;
					imm.get().gfx->own = "";
					imm.get().gfx->tid = 0;
					imm.get().gfx->inUse.store(false, std::memory_order_release);
					imm.get().gfx = nullptr;
					return;
				}
			}
		} else {
			//No graphics, can block as long as needed
			lk.lock();
		}

		//Submit buffer to queue
		vulkan->queue.submit2(submit, imm.get().fence);

		//Present if graphics is in use
		if(imm.get().gfx) {
			vk::PresentInfoKHR presentInfo(imm.get().gfx->doneRendering, vulkan->swapchain.chain, imm.get().gfx->imageIdx);
			try {
				vulkan->queue.presentKHR(presentInfo);
			} catch(vk::OutOfDateKHRError& outOfDate) {
				vulkan->swapchain.regen.store(true);
			}
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
		//Regenerate swapchain if needed
		if(vulkan->swapchain.regen.exchange(false)) {
			vulkan->dev.waitIdle();
			GenSwapchain();
		}

		//Go through all the submitted command buffers and see if they're done
		std::lock_guard lk(mutex);
		for(auto it = submitted.begin(); it != submitted.end();) {
			std::unique_ptr<VulkanCommandBuffer>& vcb = *it;
			if(vulkan->dev.getFenceStatus(vcb->imm.get().fence) == vk::Result::eSuccess) {
				//Reset the fence
				vulkan->dev.resetFences(vcb->imm.get().fence);

				//Release graphics handler if needed
				if(vcb->imm.get().gfx) {
					vcb->imm.get().gfx->imageIdx = UINT32_MAX;
					vcb->imm.get().gfx->own = "";
					vcb->imm.get().gfx->tid = 0;
					vcb->imm.get().gfx->inUse.store(false, std::memory_order_release);
					vcb->imm.get().gfx = nullptr;
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
			//Reset fence if needed
			if(vulkan->dev.getFenceStatus(imageFence) == vk::Result::eSuccess) {
				vk::Result fenceWait = vulkan->dev.waitForFences(imageFence, VK_TRUE, UINT64_MAX);
				Check<ExternalException>(fenceWait == vk::Result::eSuccess, "Failed to perform swapchain acquisition fence wait operation!");
				vulkan->dev.resetFences(imageFence);
			}

			//Acquire image
			vk::AcquireNextImageInfoKHR acquireInfo(vulkan->swapchain.chain, UINT64_MAX, acquireImage, imageFence, 1);
			auto result = vulkan->dev.acquireNextImage2KHR(acquireInfo);
			if(result.result != vk::Result::eSuccess) throw vk::SystemError(result.result, "Unknown reason.");
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
			imageIdx = UINT32_MAX;
			std::stringstream msg;
			msg << "Failed to acquire swapchain image: " << err.what();
			Check<ExternalException>(false, msg.str());
		}

		//Wait for image acquisition to finish
		vulkan->dev.waitForFences(imageFence, VK_TRUE, UINT64_MAX);
		vulkan->dev.resetFences(imageFence);
	}
}