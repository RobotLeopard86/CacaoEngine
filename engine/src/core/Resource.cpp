#include "Cacao/ResourceManager.hpp"
#include "Cacao/Cubemap.hpp"
#include "Cacao/Resource.hpp"
#include "Cacao/Sound.hpp"
#include "Cacao/World.hpp"
#include "ImplAccessor.hpp"
#include "impl/ResourceManager.hpp"
#include "SingletonGet.hpp"

#include <memory>
#include <typeindex>

namespace Cacao {
	bool BaseResAddrCheck(std::string check, std::string specialAllow = "") {
		//Restrict character set
		std::string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_:" + specialAllow;
		if(check.find_first_not_of(allowed) != std::string::npos) return false;

		//Ensure type prefix is alphabetical (ASCII magic)
		if(check[0] == '_' || check[0] <= 58) return false;

		//Ensure type and identifier separator exists
		if(check[1] != ':') return false;

		//Ensure there isn't a second identifier separator
		if(check.find_first_of(':', 2) != std::string::npos) return false;

		return true;
	}

	template<>
	bool Resource::ValidateResourceAddr<Tex2D>(const std::string& addr) {
		return BaseResAddrCheck(addr, addr[0] == 'm' ? "%" : "") && (addr[0] == 'a' || (addr[0] == 'm' && addr.find_first_of('%') != std::string::npos)) && addr.find_first_of('%') == addr.find_last_of('%');
	}

	template<>
	bool Resource::ValidateResourceAddr<Mesh>(const std::string& addr) {
		return BaseResAddrCheck(addr, addr[0] == 'm' ? "/" : "") && (addr[0] == 'a' || (addr[0] == 'm' && addr.find_first_of('/') != std::string::npos)) && addr.find_first_of('/') == addr.find_last_of('/');
	}

	template<>
	bool Resource::ValidateResourceAddr<Model>(const std::string& addr) {
		return BaseResAddrCheck(addr) && addr[0] == 'a';
	}

	template<>
	bool Resource::ValidateResourceAddr<Cubemap>(const std::string& addr) {
		return BaseResAddrCheck(addr) && addr[0] == 'a';
	}

	template<>
	bool Resource::ValidateResourceAddr<Sound>(const std::string& addr) {
		return BaseResAddrCheck(addr) && addr[0] == 'a';
	}

	template<>
	bool Resource::ValidateResourceAddr<World>(const std::string& addr) {
		return BaseResAddrCheck(addr) && addr[0] == 'w';
	}

	template<>
	bool Resource::ValidateResourceAddr<TextBlobResource>(const std::string& addr) {
		return BaseResAddrCheck(addr, "./") && addr[0] == 'r' && addr.find("..") == std::string::npos && addr.find("//") == std::string::npos && addr.find("./") == std::string::npos;
	}

	template<>
	bool Resource::ValidateResourceAddr<BinaryBlobResource>(const std::string& addr) {
		return BaseResAddrCheck(addr, "./") && addr[0] == 'r' && addr.find("..") == std::string::npos && addr.find("//") == std::string::npos && addr.find("./") == std::string::npos;
	}

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

	bool ResourceManager::IsLoaderRegistered(std::type_index tp) {
		return impl->loaders.contains(tp);
	}

	std::shared_ptr<Resource> ResourceManager::CheckCache(const std::string& addr) {
		return impl->cache.contains(addr) ? impl->cache[addr].lock() : std::shared_ptr<Resource>();
	}

	std::shared_ptr<Resource> ResourceManager::InvokeLoader(std::type_index tp, const std::string& addr) {
		Check<BadStateException>(IsLoaderRegistered(tp), "A loader has not been configured for this type!");
		return impl->loaders[tp].load(addr);
	}

	BinaryBlobResource::BinaryBlobResource(std::vector<unsigned char>&& data, const std::string& addr)
	  : BlobResource(addr), data(data) {
		Check<BadValueException>(ValidateResourceAddr<BinaryBlobResource>(addr), "Resource address is malformed!");
	}

	TextBlobResource::TextBlobResource(std::string&& data, const std::string& addr)
	  : BlobResource(addr), data(data) {
		Check<BadValueException>(ValidateResourceAddr<TextBlobResource>(addr), "Resource address is malformed!");
	}
}