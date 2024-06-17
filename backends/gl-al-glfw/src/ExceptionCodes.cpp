#include "Core/Engine.hpp"
#include "Core/Exception.hpp"

namespace Cacao {
	void Engine::RegisterBackendExceptions() {
		Exception::RegisterExceptionCode(100, "BadBindState");
		Exception::RegisterExceptionCode(101, "GLFWError");
		Exception::RegisterExceptionCode(102, "OpenGLError");
		Exception::RegisterExceptionCode(103, "UniformUploadFailure");
		Exception::RegisterExceptionCode(104, "RenderThread");
	}
}