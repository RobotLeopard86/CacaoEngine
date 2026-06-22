#include "Cacao/ResourceManager.hpp"
#include "Cacao/Cubemap.hpp"
#include "Cacao/Resource.hpp"
#include "Cacao/Sound.hpp"
#include "Cacao/World.hpp"
#include "ImplAccessor.hpp"
#include "impl/ResourceManager.hpp"
#include "SingletonGet.hpp"

#include "libcacaoasset.hpp"

#include <memory>
#include <typeindex>

namespace Cacao {
	template<>
	bool Resource::ValidateResourceAddr<Tex2D>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::Tex2D);
	}

	template<>
	bool Resource::ValidateResourceAddr<Mesh>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::Mesh);
	}

	template<>
	bool Resource::ValidateResourceAddr<Model>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::Model);
	}

	template<>
	bool Resource::ValidateResourceAddr<Cubemap>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::Cubemap);
	}

	template<>
	bool Resource::ValidateResourceAddr<Sound>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::Audio);
	}

	template<>
	bool Resource::ValidateResourceAddr<Shader>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::Shader);
	}

	template<>
	bool Resource::ValidateResourceAddr<World>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::World);
	}

	template<>
	bool Resource::ValidateResourceAddr<TextBlobResource>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::Blob);
	}

	template<>
	bool Resource::ValidateResourceAddr<BinaryBlobResource>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::Blob);
	}

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

	void ResourceManager::RegisterLoader(std::type_index tp, ResourceManager::ErasedLoader el) {
		impl->loaders.insert_or_assign(tp, el);
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