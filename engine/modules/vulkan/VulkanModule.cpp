#include "Module.hpp"

namespace Cacao {
	std::shared_ptr<PALModule> CreateVulkanModule() {
		return std::static_pointer_cast<PALModule>(std::make_shared<VulkanModule>());
	}
}