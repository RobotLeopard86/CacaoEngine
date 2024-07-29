#include "ExceptionCodes.hpp"

#include "Core/Engine.hpp"

namespace Cacao {
	void Engine::RegisterBackendExceptions() {
		RegisterGraphicsExceptions();
		RegisterWindowingExceptions();
	}
}