#include "Core/Engine.hpp"
#include "Core/Assert.hpp"
#include "Graphics/Window.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

namespace Cacao {
	NativeData* Engine::_CreateGraphicsContext() {
		return new NativeData();
	}

	void Engine::SetupGraphicsContext(NativeData* context){}

	void Engine::_DeleteGraphicsContext(NativeData* context){}
}