#include "Cacao/GPU.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Log.hpp"
#include "VulkanModule.hpp"
#include "ImplAccessor.hpp"
#include "impl/GPUManager.hpp"

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
	  : transient(std::move(other.transient)), render(std::move(other.render)), promise(std::move(other.promise)), primary(std::move(other.primary)), secondaries(std::move(other.secondaries)) {}

	VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other) {
		transient = std::move(other.transient);
		render = std::move(other.render);
		promise = std::move(other.promise);
		primary = std::move(other.primary);
		secondaries = std::move(other.secondaries);
		return *this;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer() {
		if(poolPtr == nullptr) return;

		//Free secondary command buffers
		for(vk::CommandBuffer& secondary : secondaries) {
			vulkan->dev.freeCommandBuffers(*poolPtr, secondary);
		}

		//Free primary command buffer
		vulkan->dev.freeCommandBuffers(*poolPtr, primary);
		poolPtr = nullptr;
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
			Logger::Engine(Logger::Level::Trace) << render->id << " falso (claim)";
			render->available.store(false);

			//Set semaphore done value
			render->sync.doneValue = vulkan->dev.getSemaphoreCounterValue(render->sync.semaphore) + 1;

			//Set command pool pointer
			poolPtr = &vulkan->renderingPool;

			//Acquire image
			try {
				vk::AcquireNextImageInfoKHR acquireInfo(vulkan->swapchain.chain, UINT64_MAX, render->acquire, render->fence, 1);
				auto result = vulkan->dev.acquireNextImage2KHR(acquireInfo);
				if(result.result != vk::Result::eSuccess) throw vk::SystemError(result.result, "Unknown reason.");
				render->imageIndex = result.value;
			} catch(vk::SystemError& err) {
				//Is the swapchain out of date?
				//If so, we can regenerate and try again
				if(err.code() == vk::Result::eSuboptimalKHR || err.code() == vk::Result::eErrorOutOfDateKHR || err.code() == vk::Result::eTimeout || err.code() == vk::Result::eNotReady) {
					poolPtr = nullptr;
					Logger::Engine(Logger::Level::Trace) << render->id << " truey (try again)";
					render->available.store(true);
					render = nullptr;
					GenSwapchain();
					return SetupContext(true);
				}

				//Other error, can't proceed
				render->imageIndex = UINT32_MAX;
				poolPtr = nullptr;
				Logger::Engine(Logger::Level::Trace) << render->id << " truey (oh fork)";
				render->available.store(true);
				render = nullptr;
				std::stringstream msg;
				msg << "Failed to acquire swapchain image: " << err.what();
				Check<ExternalException>(false, msg.str());
			}

			//Wait for image acquisition to finish
			vulkan->dev.waitForFences(render->fence, VK_TRUE, UINT64_MAX);
			vulkan->dev.resetFences(render->fence);
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
			Logger::Engine(Logger::Level::Trace) << render->id << " truey (no cmd)";
			render->available.store(true);
			render = nullptr;
			transient = nullptr;
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
			signals[1] = vk::SemaphoreSubmitInfo(GetSync().semaphore, GetSync().doneValue, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
		}
		vk::SubmitInfo2 submitInfo({}, wait, cbSubmit, signals);

		//If rendering and the swapchain is regenerating or about to be, this frame won't be valid, so we have to discard everything (unfortunately)
		if(render && (vulkan->swapchain.regenRequested.load(std::memory_order_relaxed) || IMPL(GPUManager).IsRegenerating())) {
			render->imageIndex = UINT32_MAX;
			Logger::Engine(Logger::Level::Trace) << render->id << " truey (gonna give you up)";
			render->available.store(true);
			poolPtr = nullptr;
			render = nullptr;
			promise.set_value();

			//We don't use Check() here because we don't need a message; this just bails us out of Submit and back to the frame processor without putting stuff in the submitted queue
			throw MiscException("Frame skip - regen in progress");
		}

		//Obtain queue lock
		std::lock_guard lk(vulkan->queueMtx);

		//Submit (and present if rendering)
		vulkan->queue.submit2(submitInfo);
		if(render) {
			vk::PresentInfoKHR presentInfo(render->render, vulkan->swapchain.chain, render->imageIndex);
			try {
				vulkan->queue.presentKHR(presentInfo);
			} catch(vk::OutOfDateKHRError&) {
				vulkan->swapchain.regenRequested.store(true);
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

		//If queue empty and swapchain regen requested, regenerate
		if(vulkan->swapchain.regenRequested.load(std::memory_order_relaxed) && submitted.empty()) {
			GenSwapchain();
		}

		//Go through all the submitted command buffers and see if they're done
		for(auto it = submitted.begin(); it != submitted.end();) {
			std::unique_ptr<VulkanCommandBuffer>& vcb = *it;
			Sync sync = vcb->GetSync();
			if(vulkan->dev.getSemaphoreCounterValue(sync.semaphore) >= sync.doneValue) {
				//Free secondary command buffers
				for(vk::CommandBuffer& secondary : vcb->secondaries) {
					vulkan->dev.freeCommandBuffers(*vcb->poolPtr, secondary);
				}

				//Mark context as available
				Logger::Engine(Logger::Level::Trace) << vcb->render->id << " truey (dun)";
				vcb->render->available.store(true);

				//Release context
				vcb->poolPtr = nullptr;
				vcb->render = nullptr;
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

	bool VulkanGPU::IsRegenerating() {
		return vulkan->swapchain.regenInProgress.load(std::memory_order_relaxed);
	}
}