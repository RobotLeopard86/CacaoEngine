#include "Core/Engine.hpp"
#include "Core/Exception.hpp"

namespace Cacao {
	void Engine::RegisterBackendExceptions() {
		Exception::RegisterExceptionCode(100, "BadCompileState");
		Exception::RegisterExceptionCode(101, "BadBindState");
		Exception::RegisterExceptionCode(102, "GLFWError");
		Exception::RegisterExceptionCode(103, "GLESError");
		Exception::RegisterExceptionCode(104, "UniformUploadFailure");
		Exception::RegisterExceptionCode(105, "RenderThread");
		Exception::RegisterExceptionCode(106, "UnsupportedType");
	}
}