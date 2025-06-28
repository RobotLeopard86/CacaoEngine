#pragma once

#include "Cacao/ResourceManager.hpp"

#include <map>

namespace Cacao {
	struct ResourceManager::Impl {
		std::map<std::string, std::weak_ptr<Resource>> cache;
	};
}