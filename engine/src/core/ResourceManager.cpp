#include "Cacao/ResourceManager.hpp"
#include "ImplAccessor.hpp"
#include "impl/ResourceManager.hpp"
#include "SingletonGet.hpp"

namespace Cacao {
	Resource::~Resource() {
		//Remove our pointer from the cache
		IMPL(ResourceManager).cache.erase(address);
	}

	CACAOST_GET(ResourceManager)

	ResourceManager::ResourceManager() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	ResourceManager::~ResourceManager() {}
}