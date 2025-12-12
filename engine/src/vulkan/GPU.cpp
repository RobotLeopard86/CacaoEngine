#include "Cacao/GPU.hpp"
#include "Cacao/Exceptions.hpp"
#include "VulkanModule.hpp"

#include <atomic>
#include <future>
#include <mutex>
#include <stdexcept>

namespace Cacao {
	std::set<std::unique_ptr<TransientCommandContext>> TransientCommandContext::contexts = {};

	TransientCommandContext* TransientCommandContext::Get() {
		static thread_local std::unique_ptr<TransientCommandContext> ctx = []() {
			std::unique_ptr<TransientCommandContext> ctx(new TransientCommandContext());
			if(!ctx->ready) {
				try {
					vk::CommandPoolCreateInfo ipoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, 0);
					ctx->pool = vulkan->dev.createCommandPool(ipoolCI);
					vk::CommandBufferAllocateInfo allocCI(ctx->pool, vk::CommandBufferLevel::ePrimary, 1);
					ctx->fence = vulkan->dev.createFence({vk::FenceCreateFlagBits::eSignaled});
					ctx->ready = true;
				} catch(...) {
					Check<ExternalException>(false, "Failed to setup transient command object object for thread!", [&ctx]() {
						if(ctx->pool) vulkan->dev.destroyCommandPool(ctx->pool);
						if(ctx->fence) vulkan->dev.destroyFence(ctx->fence);
					});
				}
			}
			return ctx;
		}();
		contexts.insert(ctx);
		return ctx.get();
	}

	void TransientCommandContext::Cleanup() {
		//Wait for the device to be idle
		vulkan->dev.waitIdle();

		for(const std::unique_ptr<TransientCommandContext>& ctx : contexts) {
			vulkan->dev.destroyCommandPool(ctx->pool);
			vulkan->dev.destroyFence(ctx->fence);
		}
		contexts.clear();
	}

	GPUManager::Impl* VulkanModule::ConfigureGPUManager() {
		return new VulkanGPU();
	}

	VulkanCommandBuffer::~VulkanCommandBuffer() {
		//Free command buffer
		vulkan->dev.freeCommandBuffers(*poolPtr, cmd);
		poolPtr = nullptr;
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other)
	  : transient(std::move(other.transient)), render(std::move(other.render)), promise(std::move(other.promise)) {}

	VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other) {
		transient = std::move(other.transient);
		render = std::move(other.render);
		promise = std::move(other.promise);
		return *this;
	}

	void VulkanCommandBuffer::Execute() {
		/*//End recording
		ctx.get().cmd.end();

		//Build submission info
		vk::SubmitInfo2 submit;
		vk::CommandBufferSubmitInfo cbsi(ctx.get().cmd);
		if(ctx.get().gfx) {
			vk::SemaphoreSubmitInfo semWait(ctx.get().gfx->acquireImage, 0, vk::PipelineStageFlagBits2::eAllCommands);
			vk::SemaphoreSubmitInfo semSignal(ctx.get().gfx->doneRendering, 0, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
			submit = vk::SubmitInfo2({}, semWait, cbsi, semSignal);
		} else {
			submit = vk::SubmitInfo2({}, {}, cbsi);
		}

		//Reset fence if needed
		if(vulkan->dev.getFenceStatus(ctx.get().fence) == vk::Result::eSuccess) {
			vk::Result fenceWait = vulkan->dev.waitForFences(ctx.get().fence, VK_TRUE, std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count());
			Check<ExternalException>(fenceWait == vk::Result::eSuccess, "Waited too long for ctxediate fence reset!");
			vulkan->dev.resetFences(ctx.get().fence);
		}

		//Obtain queue lock
		std::unique_lock<std::timed_mutex> lk(vulkan->queueMtx, std::defer_lock_t {});
		if(ctx.get().gfx) {
			//If we have graphics, try to lock for a bit at a time
			//This is to allow for swapchain regen
			while(!lk.try_lock_for(time::fmilliseconds(1))) {
				//Let's see if we're trying to regenerate the swapchain
				if(vulkan->swapchain.regen.load(std::memory_order_relaxed)) {
					//Swapchain regen cannot take place because we're blocked
					//So we'll have to give up this frame
					//Unfortuanately, that happens sometimes
					ctx.get().gfx->imageIdx = UINT32_MAX;
					ctx.get().gfx->own = "";
					ctx.get().gfx->tid = 0;
					ctx.get().gfx->inUse.store(false, std::memory_order_release);
					ctx.get().gfx = nullptr;
					promise.set_value();
					return;
				}
			}
		} else {
			//No graphics, can block as long as needed
			lk.lock();
		}

		//Submit buffer to queue
		vulkan->queue.submit2(submit, ctx.get().fence);

		//Present if graphics is in use
		if(ctx.get().gfx) {
			vk::PresentInfoKHR presentInfo(ctx.get().gfx->doneRendering, vulkan->swapchain.chain, ctx.get().gfx->imageIdx);
			try {
				vulkan->queue.presentKHR(presentInfo);
			} catch(vk::OutOfDateKHRError& outOfDate) {
				vulkan->swapchain.regen.store(true);
			}
		}*/
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
		//Lock queue
		std::lock_guard lk(mutex);

		//Go through all the submitted command buffers and see if they're done
		for(auto it = submitted.begin(); it != submitted.end();) {
			std::unique_ptr<VulkanCommandBuffer>& vcb = *it;
			if(vulkan->dev.getFenceStatus(vcb->GetFence()) == vk::Result::eSuccess) {
				//Reset the fence
				vulkan->dev.resetFences(vcb->GetFence());

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

	/*void GfxHandler::Acquire() {
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
			if(err.code() == vk::Result::eSuboptimalKHR || err.code() == vk::Result::eErrorOutOfDateKHR || err.code() == vk::Result::eTimeout) {
				vulkan->swapchain.regen.store(true, std::memory_order_seq_cst);
				GenSwapchain();
				vulkan->swapchain.regen.store(false, std::memory_order_release);
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
	}*/
}