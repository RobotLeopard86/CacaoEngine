#include "Cacao/GPU.hpp"
#include "Cacao/Exceptions.hpp"
#include "Module.hpp"
#include "Context.hpp"
#include <future>
#include <stdexcept>

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
		OpenGLCommandBuffer cmd = [this]() {
			//Lock the queue
			std::lock_guard lk(queueMtx);

			//Pop the item
			OpenGLCommandBuffer& cb = commands.front();
			commands.pop();

			//Return the buffer
			return std::move(cb);
		}();

		//Run it
		cmd.Execute();
	}

	std::shared_future<void> OpenGLGPU::SubmitCmdBuffer(const CommandBuffer& cmd) {
		//Make sure this is an OpenGL buffer
		OpenGLCommandBuffer glCmd = [&cmd]() {
			try {
				return std::move(dynamic_cast<const OpenGLCommandBuffer&>(cmd));
			} catch(...) {
				Check<BadTypeException>(false, "Cannot submit a non-OpenGL command buffer to the OpenGL backend!");
				throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
			}
		}();

		//Get the future
		std::shared_future<void> fut = glCmd.GetFuture();

		//Add the command to the queue
		{
			std::lock_guard lk(queueMtx);
			commands.push(glCmd);
		}

		//Return the future
		return fut;
	}

	GPUManager::Impl* OpenGLModule::ConfigureGPUManager() {
		return new OpenGLGPU();
	}
}