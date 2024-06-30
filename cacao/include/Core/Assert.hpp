#pragma once

#include <string>

namespace Cacao {
	void EngineAssert(bool condition, std::string errorMsg);
	void ClientAssert(bool condition, std::string errorMsg);
}