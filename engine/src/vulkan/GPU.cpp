#include "Cacao/GPU.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/FrameProcessor.hpp"
#include "VulkanModule.hpp"
#include "ImplAccessor.hpp"
#include "impl/GPUManager.hpp"
#include "impl/FrameProcessor.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <atomic>
#include <future>
#include <mutex>
#include <stdexcept>

namespace Cacao {
	std::set<TransientCommandContext*> TransientCommandContext::contexts = {};

	TransientCommandContext* TransientCommandContext::Get() {
		static thread_local std::unique_ptr<TransientCommandContext> ctx = []() {
			std::unique_ptr<TransientCommandContext> ctx(new TransientCommandContext());
			if(!ctx->ready) {
				try {
					vk::CommandPoolCreateInfo poolCI(vk::CommandPoolCreateFlagBits::eTransient, 0);
					ctx->pool = vulkan->dev.createCommandPool(poolCI);
					vk::CommandBufferAllocateInfo allocCI(ctx->pool, vk::CommandBufferLevel::ePrimary, 1);
					ctx->sync = {};
					vk::SemaphoreTypeCreateInfoKHR semTypeCI(vk::SemaphoreType::eTimeline, 0);
					ctx->sync.semaphore = vulkan->dev.createSemaphore(vk::SemaphoreCreateInfo {{}, &semTypeCI});
					ctx->ready = true;
				} catch(...) {
					Check<ExternalException>(false, "Failed to setup transient command object object for thread!", [&ctx]() {
						if(ctx->pool) vulkan->dev.destroyCommandPool(ctx->pool);
						if(ctx->sync.semaphore) vulkan->dev.destroySemaphore(ctx->sync.semaphore);
					});
				}
			}
			return ctx;
		}();
		contexts.insert(ctx.get());
		return ctx.get();
	}

	void TransientCommandContext::Cleanup() {
		//Wait for the device to be idle
		vulkan->dev.waitIdle();

		//Destroy all context objects
		for(TransientCommandContext* ctx : contexts) {
			vulkan->dev.destroyCommandPool(ctx->pool);
			vulkan->dev.destroySemaphore(ctx->sync.semaphore);
		}
		contexts.clear();
	}

	GPUManager::Impl* VulkanModule::ConfigureGPUManager() {
		return new VulkanGPU();
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other)
	  : cmd(std::move(other.cmd)), transient(std::exchange(other.transient, nullptr)), render(std::exchange(other.render, nullptr)), imageCtx(std::exchange(other.imageCtx, nullptr)), poolPtr(std::exchange(other.poolPtr, nullptr)), promise(std::move(other.promise)) {}

	VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other) {
		if(this == &other) return *this;

		cmd = std::move(other.cmd);
		transient = std::exchange(other.transient, nullptr);
		render = std::exchange(other.render, nullptr);
		imageCtx = std::exchange(other.imageCtx, nullptr);
		promise = std::move(other.promise);
		poolPtr = std::exchange(other.poolPtr, nullptr);

		return *this;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer() {
		if(poolPtr == nullptr) return;

		//If this was a rendering context, release context
		if(render) render = nullptr;
		if(transient) transient = nullptr;

		//Free command buffer
		vulkan->dev.freeCommandBuffers(*poolPtr, cmd);
		poolPtr = nullptr;
	}

	bool VulkanCommandBuffer::SetupContext(bool rendering) {
		//Obtain context object
		if(rendering) {
			//Get the next contexts and advance the cycle
			render = vulkan->swapchain.renderContexts[vulkan->swapchain.cycle].get();
			imageCtx = vulkan->swapchain.imageContexts[vulkan->swapchain.cycle].get();
			vulkan->swapchain.cycle = ++vulkan->swapchain.cycle % vulkan->swapchain.renderContexts.size();

			//Wait until render context is available
			vk::SemaphoreWaitInfo waitInfo({}, render->sync.semaphore, render->sync.doneValue);
			vulkan->dev.waitSemaphores(waitInfo, UINT64_MAX);

			//Increment semaphore done value
			++(render->sync.doneValue);

			//Set command pool pointer
			poolPtr = &vulkan->renderingPool;

			//Acquire image
			uint32_t imageIndex = UINT32_MAX;
			try {
				vk::AcquireNextImageInfoKHR acquireInfo(vulkan->swapchain.chain, UINT64_MAX, imageCtx->acquire, VK_NULL_HANDLE, 1);
				auto result = vulkan->dev.acquireNextImage2KHR(acquireInfo);
				if(result.result != vk::Result::eSuccess) throw vk::SystemError(result.result, "Unknown reason.");
				imageIndex = result.value;
			} catch(vk::SystemError& err) {
				//Reset data members
				poolPtr = nullptr;
				render = nullptr;
				imageCtx = nullptr;

				//Is the swapchain out of date?
				//If so, we can regenerate and try again
				if(err.code() == vk::Result::eSuboptimalKHR || err.code() == vk::Result::eErrorOutOfDateKHR || err.code() == vk::Result::eTimeout || err.code() == vk::Result::eNotReady) {
					Event e("INTERNAL-RegenSwapchain");
					EventManager::Get().Dispatch(e);
					return false;
				}

				//Other error, can't proceed safely
				std::stringstream msg;
				msg << "Failed to acquire swapchain image: " << err.what();
				Check<ExternalException>(false, msg.str());
			}

			//Set image index
			render->imageIndex = imageIndex;
		} else {
			//Get context and set pool pointer
			transient = TransientCommandContext::Get();
			poolPtr = &transient->pool;

			//Set semaphore done value
			++(transient->sync.doneValue);
		}

		//Create command buffer from pool
		try {
			vk::CommandBufferAllocateInfo allocInfo(*poolPtr, vk::CommandBufferLevel::ePrimary, 1);
			cmd = vulkan->dev.allocateCommandBuffers(allocInfo)[0];
		} catch(...) {
			poolPtr = nullptr;
			if(render) {
				render->imageIndex = UINT32_MAX;
				render = nullptr;
			} else {
				transient = nullptr;
			}
			return false;
		}

		//Begin command buffer recording
		vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmd.begin(beginInfo);

		return true;
	}

	Sync VulkanCommandBuffer::GetSync() {
		if(render) return render->sync;
		if(transient) return transient->sync;
		return {};
	}

	void VulkanCommandBuffer::Execute() {
		//If rendering, we use secondary command buffers for each command; we need to execute those within the primary command buffer
		if(render) {
			//Make our image presentable
			{
				vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite,
					vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eNone,
					vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, 0, 0, vulkan->swapchain.images[render->imageIndex],
					vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
				vk::DependencyInfo transition({}, {}, {}, barrier);
				cmd.pipelineBarrier2(transition);
			}

			//Put the depth image into a read-only format to not leave it in a rendering state
			{
				vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllGraphics, vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
					vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eNone,
					vk::ImageLayout::eDepthAttachmentOptimal, vk::ImageLayout::eDepthReadOnlyOptimal, 0, 0, vulkan->swapchain.depthImages[render->imageIndex].obj,
					vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
				vk::DependencyInfo transition({}, {}, {}, barrier);
				cmd.pipelineBarrier2(transition);
			}
		}

		//End primary command buffer recording
		cmd.end();

		//Build submission info
		vk::CommandBufferSubmitInfo cbSubmit(cmd);
		vk::SemaphoreSubmitInfo wait = {};
		std::array<vk::SemaphoreSubmitInfoKHR, 2> signals = {};
		if(render) {
			wait = vk::SemaphoreSubmitInfo(imageCtx->acquire, 0, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
			signals[0] = vk::SemaphoreSubmitInfo(imageCtx->render, 0, vk::PipelineStageFlagBits2::eAllCommands);
			signals[1] = vk::SemaphoreSubmitInfo(GetSync().semaphore, GetSync().doneValue, vk::PipelineStageFlagBits2::eAllCommands);
		}
		vk::SubmitInfo2 submitInfo({}, wait, cbSubmit, signals);

		//Obtain queue lock
		std::lock_guard lk(vulkan->queueMtx);

		//Submit (and present if rendering)
		vulkan->queue.submit2(submitInfo);
		if(render) {
			vk::PresentInfoKHR presentInfo(imageCtx->render, vulkan->swapchain.chain, render->imageIndex);
			try {
				vulkan->queue.presentKHR(presentInfo);
			} catch(vk::OutOfDateKHRError&) {
				Event e("INTERNAL-RegenSwapchain");
				EventManager::Get().Dispatch(e);
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
				throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's a toddler
			}
		}();

		//Lock queue
		std::lock_guard lk(mutex);

		//Submit underlying commands
		vkCmd->Execute();

		//Get a future
		std::shared_future<void> fut = vkCmd->promise.get_future().share();

		//Move buffer into submission list for result tracking
		submitted.push_back(std::move(vkCmd));

		//Return future
		return fut;
	}

	void VulkanGPU::RunloopIteration() {
		//Lock queue
		std::lock_guard lk(mutex);

		//Go through all the submitted command buffers and see if they're done
		for(auto it = submitted.begin(); it != submitted.end();) {
			std::unique_ptr<VulkanCommandBuffer>& vcb = *it;
			Sync sync = vcb->GetSync();
			if(vulkan->dev.getSemaphoreCounterValue(sync.semaphore) >= sync.doneValue) {
				//If this was a rendering context, mark it available and adjust counter
				if(vcb->render) {
					--(IMPL(FrameProcessor).numFramesInFlight);
					vcb->render = nullptr;
					vcb->imageCtx = nullptr;
				}

				//Release context pointers
				vcb->poolPtr = nullptr;
				vcb->transient = nullptr;

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

	void VulkanGPU::RunloopStop() {
		//Wait until all jobs are done and clean them up
		//To avoid code duplication, we just call Iteration over and over
		while(submitted.size() > 0) {
			RunloopIteration();
		}
		vulkan->dev.waitIdle();
	}

	unsigned int VulkanGPU::MaxFramesInFlight() {
		return vulkan->swapchain.renderContexts.size();
	}
}