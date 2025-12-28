#include "Cacao/Event.hpp"
#include "OpenGLModule.hpp"
#include "Cacao/GPU.hpp"
#include "Context.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/EventManager.hpp"
#include "Cacao/PAL.hpp"
#include "ImplAccessor.hpp"

#include "glad/gl.h"

namespace Cacao {
	struct OpenGLModuleRegistrar {
		OpenGLModuleRegistrar() {
			IMPL(PAL).registry.insert_or_assign("opengl", []() { gl = std::make_shared<OpenGLModule>(); return gl; });
		}
	};
	__attribute__((used)) OpenGLModuleRegistrar glmr;

	void OpenGLModule::Init() {
		didInit = true;
	}
	void OpenGLModule::Term() {
		didInit = false;
	}
	void OpenGLModule::Connect() {
		//Make context and configure
		ctx = std::make_shared<Context>();
		glm::uvec2 contentSize = Window::Get().GetContentAreaSize();
		glViewport(0, 0, contentSize.x, contentSize.y);

		//Print OpenGL info
		ctx->MakeCurrent();
		const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
		const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		Logger::Engine(Logger::Level::Trace) << "OpenGL v" << version << ", using " << renderer << " (" << vendor << ")";
		ctx->Yield();

		//Print OpenGL info
		ctx->MakeCurrent();
		const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
		const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		Logger::Engine(Logger::Level::Trace) << "OpenGL v" << version << ", using " << renderer << " (" << vendor << ")";
		ctx->Yield();

		connected = true;
	}

	void OpenGLModule::Disconnect() {
		connected = false;

		//Destroy context
		ctx.reset();
	}

	void OpenGLModule::Destroy() {
		gl.reset();
	}

	void OpenGLModule::SetVSync(bool state) {
		ctx->SetVSync(state);
	}

	std::unique_ptr<CommandBuffer> OpenGLModule::CreateCmdBuffer() {
		return std::make_unique<OpenGLCommandBuffer>();
	}
}