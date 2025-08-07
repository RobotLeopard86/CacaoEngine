#pragma once

#include "Cacao/GPU.hpp"
#include "impl/PAL.hpp"
#include "impl/GPUManager.hpp"
#include "Cacao/EventConsumer.hpp"
#include "Cacao/Exceptions.hpp"

#include <sstream>
#include <future>
#include <mutex>

#include "eternal.hpp"
#include "glad/gl.h"

constexpr const inline auto glErrors = mapbox::eternal::map<GLenum, mapbox::eternal::string>({{GL_INVALID_ENUM, "Invalid Enum"},
	{GL_INVALID_VALUE, "Invalid Value"},
	{GL_INVALID_OPERATION, "Invalid Operation"},
	{GL_INVALID_FRAMEBUFFER_OPERATION, "Invalid Framebuffer"},
	{GL_OUT_OF_MEMORY, "Out of Memory"},
	{GL_STACK_OVERFLOW, "Stack Overflow"},
	{GL_STACK_UNDERFLOW, "Stack Underflow"}});

#define GL_CHECK(msg)                                                                                                    \
	if(GLenum err = glGetError(); err != GL_NO_ERROR) {                                                                  \
		std::stringstream message;                                                                                       \
		message << "OpenGL error 0x" << std::hex << err << std::dec << " (" << glErrors.at(err).c_str() << "), " << msg; \
		Check<ExternalException>(false, message.str());                                                                  \
	}

namespace Cacao {
	class OpenGLCommandBuffer final : public CommandBuffer {
	  public:
		OpenGLCommandBuffer(std::function<void()> task)
		  : task(task), promise() {}

		void Execute() override {
			try {
				task();
				promise.set_value();
			} catch(...) {
				promise.set_exception(std::current_exception());
			}
		}

		OpenGLCommandBuffer(const OpenGLCommandBuffer& other)
		  : task(other.task), promise() {}

		std::shared_future<void> GetFuture() {
			return promise.get_future().share();
		}

	  private:
		std::function<void()> task;
		std::promise<void> promise;
	};

	class OpenGLGPU final : public GPUManager::Impl {
	  public:
		std::shared_future<void> SubmitCmdBuffer(CommandBuffer&& cmd) override;
		void RunloopStart() override;
		void RunloopStop() override;
		void RunloopIteration() override;

	  private:
		std::queue<OpenGLCommandBuffer> commands;
		std::mutex queueMtx;
	};

	class OpenGLModule final : public PALModule {
	  public:
		void Init() override;
		void Term() override;
		void Connect() override;
		void Disconnect() override;
		void Destroy() override;
		void SetVSync(bool state) override;

		//==================== IMPL POINTER CONFIGURATION ====================
		virtual Mesh::Impl* ConfigureMesh() override;
		virtual Tex2D::Impl* ConfigureTex2D() override;
		virtual Cubemap::Impl* ConfigureCubemap() override;
		virtual GPUManager::Impl* ConfigureGPUManager() override;

		OpenGLModule()
		  : PALModule("opengl") {}
		~OpenGLModule() {}

	  private:
		EventConsumer resizer;
	};

	inline std::shared_ptr<OpenGLModule> gl;
}