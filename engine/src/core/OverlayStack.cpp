#include "Cacao/OverlayStack.hpp"
#include "SingletonGet.hpp"

#include "crossguid/guid.hpp"

#include <map>
#include <memory>

namespace Cacao {
	struct OverlayStack::Impl {
		std::map<std::string, PackageDescriptor> pkgs;
		std::vector<std::string> stack;
	};

	CACAOST_GET(OverlayStack)

	OverlayStack::OverlayStack() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
	}

	OverlayStack::~OverlayStack() {}

	xg::Guid OverlayStack::ResolveResourceAddr(const std::string& addr) {
		//TODO: Get highest package providing address
		std::string srcPkg = "lobster";
		return ResolveResourceAddr_Real(addr, srcPkg);
	}

	xg::Guid OverlayStack::ResolveResourceAddr_Real(const std::string& addr, const std::string& pkgId) {
		//TODO: Fetch ID
		return xg::Guid {};
	}
}