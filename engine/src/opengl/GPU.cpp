#include "Cacao/GPU.hpp"
#include "Cacao/Exceptions.hpp"
#include "OpenGLModule.hpp"
#include "Context.hpp"

#include <future>
#include <memory>
#include <stdexcept>
#include <optional>

namespace Cacao {
	void OpenGLGPU::RunloopStart() {
		//Make the context current on our thread
		ctx->MakeCurrent();
	}

	void OpenGLGPU::RunloopStop() {
		//Yield the context back to the main thread
		ctx->Yield();
	}

	void OpenGLGPU::RunloopIteration() {
		//Pop the next task off the queue
		std::optional<std::unique_ptr<OpenGLCommandBuffer>> cmd = [this]() -> std::optional<std::unique_ptr<OpenGLCommandBuffer>> {
			//Lock the queue
			std::lock_guard lk(queueMtx);

			//Check if queue is empty
			if(commands.empty()) return std::nullopt;

			//Pop the item
			std::unique_ptr<OpenGLCommandBuffer> cb = std::move(commands.front());
			commands.pop();

			//Return the buffer
			return std::move(cb);
		}();

		//Make sure we got a command
		if(!cmd.has_value()) return;

		//Run it
		cmd.value()->Execute();
	}

	std::shared_future<void> OpenGLGPU::SubmitCmdBuffer(std::unique_ptr<CommandBuffer>&& cmd) {
		//Make sure this is an OpenGL buffer
		std::unique_ptr<OpenGLCommandBuffer> glCmd = [&cmd]() -> std::unique_ptr<OpenGLCommandBuffer> {
			if(OpenGLCommandBuffer* glcb = dynamic_cast<OpenGLCommandBuffer*>(cmd.release())) {
				return std::unique_ptr<OpenGLCommandBuffer>(glcb);
			} else {
				Check<BadTypeException>(false, "Cannot submit a non-OpenGL command buffer to the OpenGL backend!");
				throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
			}
		}();

		//Get the future
		std::shared_future<void> fut = glCmd->GetFuture();

		//Add the command to the queue
		{
			std::lock_guard lk(queueMtx);
			commands.push(std::move(glCmd));
		}

		//Return the future
		return fut;
	}

	GPUManager::Impl* OpenGLModule::ConfigureGPUManager() {
		return new OpenGLGPU();
	}

	std::function<void(CommandBuffer*)>&& CommandBuffer::GetCommandFn(GPUCommand&& cmd) {
		return std::move(cmd.apply);
	}
}