#include "Cacao/GPU.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/FrameProcessor.hpp"
#include "Cacao/Log.hpp"
#include "VulkanModule.hpp"
#include "ImplAccessor.hpp"
#include "impl/GPUManager.hpp"
#include "impl/FrameProcessor.hpp"

#include <atomic>
#include <future>
#include <mutex>
#include <stdexcept>

namespace Cacao {
	std::set<TransientCommandContext*> TransientCommandContext::contexts = {};
	unsigned int VulkanCommandBuffer::acquireCount = 0;

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
	  : transient(std::exchange(other.transient, nullptr)), render(std::exchange(other.render, nullptr)), didStartRender(std::exchange(other.didStartRender, false)), poolPtr(std::exchange(other.poolPtr, nullptr)), promise(std::move(other.promise)), primary(std::move(other.primary)), secondaries(std::move(other.secondaries)) {}

	VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other) {
		if(this == &other) return *this;

		transient = std::exchange(other.transient, nullptr);
		render = std::exchange(other.render, nullptr);
		if(render) Logger::Engine(Logger::Level::Trace) << render->id << " movey";
		promise = std::move(other.promise);
		primary = std::move(other.primary);
		secondaries = std::move(other.secondaries);
		didStartRender = std::exchange(other.didStartRender, false);
		poolPtr = std::exchange(other.poolPtr, nullptr);

		return *this;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer() {
		if(poolPtr == nullptr) return;

		//If this was a rendering context, mark it available and adjust counter
		if(render) {
			//Unsignal acquire semaphore
			UnsignalAcquire();

			//Release context
			render->available.store(true);
			if(acquireCount != 0) --(acquireCount);
			render = nullptr;
		}
		if(transient) transient = nullptr;

		//Free secondary command buffers
		for(vk::CommandBuffer& secondary : secondaries) {
			vulkan->dev.freeCommandBuffers(*poolPtr, secondary);
		}

		//Free primary command buffer
		vulkan->dev.freeCommandBuffers(*poolPtr, primary);
		poolPtr = nullptr;
	}

	void VulkanCommandBuffer::UnsignalAcquire() {
		Sync sync = GetSync();
		vk::SemaphoreSubmitInfo wait(render->acquire, 0, vk::PipelineStageFlagBits2::eAllCommands);
		vk::SemaphoreSubmitInfo signal(GetSync().semaphore, sync.doneValue, vk::PipelineStageFlagBits2::eAllCommands);
		vk::SubmitInfo2 emptySubmit({}, wait, {}, signal);
		{
			std::lock_guard lk(vulkan->queueMtx);
			vulkan->queue.submit2(emptySubmit);
		}
		vk::SemaphoreWaitInfo emptyWait({}, sync.semaphore, sync.doneValue);
		vulkan->dev.waitSemaphores(emptyWait, UINT64_MAX);
	}

	vk::CommandBuffer& VulkanCommandBuffer::vk() {
		if(render) {
			//If rendering, allocate a new secondary command buffer
			//This makes the command recording system more modular
			try {
				//Create buffer
				vk::CommandBufferAllocateInfo allocInfo(*poolPtr, vk::CommandBufferLevel::eSecondary, 1);
				vk::CommandBuffer secondary = vulkan->dev.allocateCommandBuffers(allocInfo)[0];

				//Begin recording
				vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit | vk::CommandBufferUsageFlagBits::eRenderPassContinue, &vulkan->cbInheritance);
				secondary.begin(beginInfo);

				//Add to secondaries list
				secondaries.push_back(std::move(secondary));
				return secondaries[secondaries.size() - 1];
			} catch(...) {
				Check<ExternalException>(false, "Failed to allocate secondary command buffer!");
				throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's a toddler
			}
		} else {
			//Otherwise, return the primary buffer
			return primary;
		}
	}

	bool VulkanCommandBuffer::SetupContext(bool rendering) {
		//Obtain context object
		if(rendering) {
			//Ensure "forward progress" is being made (https://docs.vulkan.org/spec/latest/chapters/VK_KHR_surface/wsi.html#swapchain-acquire-forward-progress)
			if(acquireCount > (vulkan->swapchain.images.size() - vulkan->capabilities.minImageCount)) {
				//Forward progress requirement not satisifed, skip
				return false;
			}

			//Get the next context and advance the cycle
			render = vulkan->swapchain.renderContexts[vulkan->swapchain.cycle].get();
			render->id = vulkan->swapchain.cycle;
			vulkan->swapchain.cycle = ++vulkan->swapchain.cycle % vulkan->swapchain.renderContexts.size();

			//Is the context available?
			if(!render->available.load()) {
				//No, skip frame
				render = nullptr;
				return false;
			}

			//Claim context
			render->available.store(false);

			//Set semaphore done value
			render->sync.doneValue = vulkan->dev.getSemaphoreCounterValue(render->sync.semaphore) + 1;

			//Set command pool pointer
			poolPtr = &vulkan->renderingPool;

			//Acquire image
			try {
				vk::AcquireNextImageInfoKHR acquireInfo(vulkan->swapchain.chain, UINT64_MAX, render->acquire, VK_NULL_HANDLE, 1);
				auto result = vulkan->dev.acquireNextImage2KHR(acquireInfo);
				if(result.result != vk::Result::eSuccess) throw vk::SystemError(result.result, "Unknown reason.");
				render->imageIndex = result.value;
				++acquireCount;
			} catch(vk::SystemError& err) {
				//Is the swapchain out of date?
				//If so, we can regenerate and try again
				if(err.code() == vk::Result::eSuboptimalKHR || err.code() == vk::Result::eErrorOutOfDateKHR || err.code() == vk::Result::eTimeout || err.code() == vk::Result::eNotReady) {
					//Unsignal acquire semaphore if we got suboptimal result
					//This is because suboptimal means we still got an image
					if(err.code() == vk::Result::eSuboptimalKHR) UnsignalAcquire();

					//Reset data members, regen swapchain, and try again
					poolPtr = nullptr;
					render->available.store(true);
					render = nullptr;
					return false;
				}

				//Other error, can't proceed
				render->imageIndex = UINT32_MAX;
				poolPtr = nullptr;
				render->available.store(true);
				render = nullptr;
				std::stringstream msg;
				msg << "Failed to acquire swapchain image: " << err.what();
				Check<ExternalException>(false, msg.str());
			}
		} else {
			//Get context and set pool pointer
			transient = TransientCommandContext::Get();
			poolPtr = &transient->pool;

			//Set semaphore done value
			transient->sync.doneValue = vulkan->dev.getSemaphoreCounterValue(transient->sync.semaphore) + 1;
		}

		//Create command buffer from pool
		try {
			vk::CommandBufferAllocateInfo allocInfo(*poolPtr, vk::CommandBufferLevel::ePrimary, 1);
			primary = vulkan->dev.allocateCommandBuffers(allocInfo)[0];
		} catch(...) {
			poolPtr = nullptr;
			if(render) {
				UnsignalAcquire();
				render->imageIndex = UINT32_MAX;
				render->available.store(true);
				if(VulkanCommandBuffer::acquireCount != 0) --(VulkanCommandBuffer::acquireCount);
				render = nullptr;
			} else {
				transient = nullptr;
			}
			return false;
		}

		//Begin primary command buffer recording
		vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		primary.begin(beginInfo);

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
			if(secondaries.size() > 0) primary.executeCommands(secondaries);
			if(didStartRender) {
				//Now we end rendering on the primary
				//This is in the if-clause as a fail-safe in case StartRendering didn't work or something
				//Realistically this check will be meaningless usually

				//End rendering
				primary.endRendering();

				//Make our image presentable
				{
					vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
						vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
						vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, 0, 0, vulkan->swapchain.images[render->imageIndex],
						vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
					vk::DependencyInfo transition({}, {}, {}, barrier);
					primary.pipelineBarrier2(transition);
				}

				//Put the depth image into a read-only format to not leave it in a rendering state
				{
					vk::ImageMemoryBarrier2 barrier(vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryWrite,
						vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
						vk::ImageLayout::eDepthAttachmentOptimal, vk::ImageLayout::eDepthReadOnlyOptimal, 0, 0, vulkan->depth.obj,
						vk::ImageSubresourceRange {vk::ImageAspectFlagBits::eDepth, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS});
					vk::DependencyInfo transition({}, {}, {}, barrier);
					primary.pipelineBarrier2(transition);
				}
			}
		}

		//End primary command buffer recording
		primary.end();

		//Build submission info
		vk::CommandBufferSubmitInfo cbSubmit(primary);
		vk::SemaphoreSubmitInfo wait = {};
		std::array<vk::SemaphoreSubmitInfoKHR, 2> signals = {};
		if(render) {
			wait = vk::SemaphoreSubmitInfo(render->acquire, 0, vk::PipelineStageFlagBits2::eAllCommands);
			signals[0] = vk::SemaphoreSubmitInfo(render->render, 0, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
			signals[1] = vk::SemaphoreSubmitInfo(GetSync().semaphore, GetSync().doneValue, vk::PipelineStageFlagBits2::eAllCommands);
		}
		vk::SubmitInfo2 submitInfo({}, wait, cbSubmit, signals);

		//Obtain queue lock
		std::lock_guard lk(vulkan->queueMtx);

		//Submit (and present if rendering)
		vulkan->queue.submit2(submitInfo);
		if(render) {
			vk::PresentInfoKHR presentInfo(render->render, vulkan->swapchain.chain, render->imageIndex);
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
				//Free secondary command buffers if we have a valid pool
				if(vcb->poolPtr) {
					for(vk::CommandBuffer& secondary : vcb->secondaries) {
						vulkan->dev.freeCommandBuffers(*vcb->poolPtr, secondary);
					}
				}

				//If this was a rendering context, mark it available and adjust counter
				if(vcb->render) {
					--(IMPL(FrameProcessor).numFramesInFlight);
					vcb->render->available.store(true);
					if(VulkanCommandBuffer::acquireCount != 0) --(VulkanCommandBuffer::acquireCount);
					vcb->render = nullptr;
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