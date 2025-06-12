#include "Cacao/ResourceManager.hpp"
#include "Cacao/OverlayStack.hpp"
#include "ImplAccessor.hpp"
#include "ResourceMgrImpl.hpp"
#include "SingletonGet.hpp"

namespace Cacao {
	std::shared_ptr<Resource> GetRPTr(const std::string& addr) {
		return IMPL(ResourceManager).cache.at(OverlayStack::Get().ResolveResourceAddr(addr)).lock();
	}

	Resource::~Resource() {
		//Remove our pointer from the cache
		xg::Guid lookupId = OverlayStack::Get().ResolveResourceAddr_Real(address, pkg);
		IMPL(ResourceManager).cache.erase(lookupId);
	}

	CACAOST_GET(ResourceManager)

	ResourceManager::ResourceManager() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	ResourceManager::~ResourceManager() {}
}