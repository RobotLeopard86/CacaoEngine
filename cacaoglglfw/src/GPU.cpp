#include "Core/Engine.hpp"
#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"

#include "GLContextData.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

namespace Cacao {
	NativeData* Engine::_CreateGraphicsContext() {
		EngineAssert(Window::GetInstance()->IsOpen(), "Window must be open to create a graphics context!");

		GLContextData* data = new GLContextData();

		//Set window initialization hints
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
#endif
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		
		//Create an invisible window that shares resources with the main window context
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		data->window = glfwCreateWindow(1, 1, "", NULL, (GLFWwindow*)Window::GetInstance()->GetNativeWindow());

		//Load OpenGL into this context
		glfwMakeContextCurrent(data->window);
		int gladResult = gladLoadGL(glfwGetProcAddress);
		EngineAssert(gladResult != 0, "Could not load Glad in graphics context creation!");

		//Release context for usage by the requester
		glfwMakeContextCurrent(NULL);

		return data;
	}

	void Engine::SetupGraphicsContext(NativeData* context){
		glfwMakeContextCurrent(((GLContextData*)context)->window);
	}

	void Engine::_DeleteGraphicsContext(NativeData* context){
		GLContextData* cd = (GLContextData*)context;
		glfwDestroyWindow(cd->window);
	}
}