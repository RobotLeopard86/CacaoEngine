#include "Cacao/ResourceManager.hpp"
#include "ImplAccessor.hpp"
#include "impl/ResourceManager.hpp"
#include "SingletonGet.hpp"

#include <memory>

namespace Cacao {
	Resource::~Resource() {
		//Remove our pointer from the cache
		IMPL(ResourceManager).cache.erase(address);
	}

	void Resource::RegisterSelf() {
		//Get our pointer
		std::shared_ptr<Resource> selfPtr = shared_from_this();

		//Cache it
		IMPL(ResourceManager).cache.insert_or_assign(address, selfPtr);
	}

	CACAOST_GET(ResourceManager)

	ResourceManager::ResourceManager() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	ResourceManager::~ResourceManager() {}
}