#pragma once

#include "Cacao/ResourceManager.hpp"

#include <map>

namespace Cacao {
	struct ResourceManager::Impl {
		std::map<xg::Guid, std::weak_ptr<Resource>> cache;
	};
}