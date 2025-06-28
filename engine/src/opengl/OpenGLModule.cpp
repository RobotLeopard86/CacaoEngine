#include "Module.hpp"
#include "Context.hpp"
#include "Cacao/Window.hpp"
#include "Cacao/PAL.hpp"
#include "ImplAccessor.hpp"
#ifdef __linux__
#include "LinuxRouter.hpp"
#endif
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

		//Register viewport resize consumer
		resizer = EventConsumer([](Event&) {
			glm::uvec2 contentSize = Window::Get().GetContentAreaSize();
			glViewport(0, 0, contentSize.x, contentSize.y);
		});
		EventManager::Get().SubscribeConsumer("WindowResize", resizer);

		connected = true;
	}
	void OpenGLModule::Disconnect() {
		connected = false;

		//Unsubscribe viewport resize consumer
		EventManager::Get().UnsubscribeConsumer("WindowResize", resizer);

		//Destroy context
		ctx.reset();
	}
	void OpenGLModule::Destroy() {
		gl.reset();
	}
	void OpenGLModule::SetVSync(bool state) {
		ctx->SetVSync(state);
	}
}