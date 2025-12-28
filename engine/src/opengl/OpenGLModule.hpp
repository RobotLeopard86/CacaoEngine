#pragma once

#include "Cacao/GPU.hpp"
#include "impl/PAL.hpp"
#include "impl/GPUManager.hpp"
#include "Cacao/Exceptions.hpp"// IWYU pragma: keep

#include <queue>
#include <future>
#include <mutex>

#include "eternal.hpp"
#include "glad/gl.h"// IWYU pragma: export

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
		OpenGLCommandBuffer()
		  : tasks(), promise() {}

		void Execute() override {
			try {
				for(auto& task : tasks) task();
				promise.set_value();
			} catch(...) {
				promise.set_exception(std::current_exception());
			}
		}

		OpenGLCommandBuffer(OpenGLCommandBuffer&& other)
		  : tasks(std::exchange(other.tasks, {})), promise(std::exchange(other.promise, {})) {}

		OpenGLCommandBuffer& operator=(OpenGLCommandBuffer&& other) {
			tasks = std::exchange(other.tasks, {});
			promise = std::exchange(other.promise, {});
			return *this;
		}

		std::shared_future<void> GetFuture() {
			return promise.get_future().share();
		}

		void AddTask(std::function<void()> task) {
			tasks.push_back(task);
		}

	  protected:
		void StartRendering(glm::vec3 clearColor) override;
		void EndRendering() override;

	  private:
		std::vector<std::function<void()>> tasks;
		std::promise<void> promise;
	};

	class OpenGLGPU final : public GPUManager::Impl {
	  public:
		std::shared_future<void> SubmitCmdBuffer(std::unique_ptr<CommandBuffer>&& cmd) override;
		void RunloopStart() override;
		void RunloopStop() override;
		void RunloopIteration() override;
		bool UsesImmediateExecution() override {
			return true;
		}
		void GenSwapchain() override;

	  private:
		std::queue<std::unique_ptr<OpenGLCommandBuffer>> commands;
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
		std::unique_ptr<CommandBuffer> CreateCmdBuffer() override;

		//==================== IMPL POINTER CONFIGURATION ====================
		Mesh::Impl* ConfigureMesh() override;
		Tex2D::Impl* ConfigureTex2D() override;
		Cubemap::Impl* ConfigureCubemap() override;
		GPUManager::Impl* ConfigureGPUManager() override;

		OpenGLModule()
		  : PALModule("opengl") {}
		~OpenGLModule() {}
	};

	inline std::shared_ptr<OpenGLModule> gl;
}