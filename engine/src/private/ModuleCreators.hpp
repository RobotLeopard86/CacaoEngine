#pragma once

#include <memory>

#include "PALCommon.hpp"

namespace Cacao {
	std::shared_ptr<PALModule> CreateVulkanModule();
	std::shared_ptr<PALModule> CreateOpenGLModule();
}