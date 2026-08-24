#include "OpenGLModule.hpp"
#include "Cacao/GPU.hpp"
#include "Cacao/Log.hpp"
#include "Context.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/PAL.hpp"
#include "ImplAccessor.hpp"

#include "glad/gl.h"
#undef Yield

namespace Cacao {
	struct OpenGLModuleRegistrar {
		OpenGLModuleRegistrar() {
			IMPL(PAL).registry.insert_or_assign("opengl", []() { gl = std::make_shared<OpenGLModule>(); return gl; });
		}
	};
	__attribute__((used)) OpenGLModuleRegistrar glmr;

#if defined(_DEBUG) && !defined(__APPLE__)
	void GLAPIENTRY OpenGLMsgCallback(GLenum source, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar* message, const void*) {
		Logger::Level lvl;
		switch(severity) {
			case GL_DEBUG_SEVERITY_MEDIUM:
				lvl = Logger::Level::Warn;
				break;
			case GL_DEBUG_SEVERITY_HIGH:
				lvl = Logger::Level::Error;
				break;
			default: return;
		}
		std::string srcStr, typeStr;
		switch(source) {
			case GL_DEBUG_SOURCE_API:
				srcStr = "API";
				break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				srcStr = "window system";
				break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				srcStr = "compiler";
				break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				srcStr = "third-party source";
				break;
			case GL_DEBUG_SOURCE_APPLICATION:
				srcStr = "app";
				break;
			case GL_DEBUG_SOURCE_OTHER:
				srcStr = "unknown source";
				break;
		}
		switch(type) {
			case GL_DEBUG_TYPE_ERROR:
				typeStr = "Error";
				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				typeStr = "Deprecation";
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				typeStr = "UB";
				break;
			case GL_DEBUG_TYPE_PORTABILITY:
				typeStr = "Portability";
				break;
			case GL_DEBUG_TYPE_PERFORMANCE:
				typeStr = "Performance";
				break;
			case GL_DEBUG_TYPE_MARKER:
				typeStr = "Annotation";
				break;
			case GL_DEBUG_TYPE_OTHER:
				typeStr = "Unknown";
				break;
		}
		Logger::Engine(lvl).LogFormatted("(OpenGL) {} message from {}: {}", typeStr, srcStr, message);
	}
#endif

	void OpenGLModule::Init() {
		didInit = true;
	}
	void OpenGLModule::Term() {
		didInit = false;
	}
	void OpenGLModule::Connect() {
		//Make context
		ctx = std::make_shared<Context>();
		ctx->MakeCurrent();

		//Configure API
		glm::uvec2 caSize = Window::Get().GetContentAreaSize();
		glViewport(0, 0, caSize.x, caSize.y);
#if defined(_DEBUG) && !defined(__APPLE__)
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMsgCallback, nullptr);
#endif

		//Create globals UBO
		glGenBuffers(1, &globalsUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, globalsUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(GlobalsData), nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//Print OpenGL info
		const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
		const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		Logger::Engine(Logger::Level::Trace) << "OpenGL v" << version << ", using " << renderer << " (" << vendor << ")";
		ctx->Yield();

		connected = true;
	}

	void OpenGLModule::Disconnect() {
		connected = false;

		//Destroy globals UBO
		glDeleteBuffers(1, &globalsUBO);

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