#include "Cacao/ResourceManager.hpp"
#include "Cacao/Cubemap.hpp"
#include "Cacao/Resource.hpp"
#include "Cacao/Sound.hpp"
#include "Cacao/Tex2D.hpp"
#include "Cacao/World.hpp"
#include "ImplAccessor.hpp"
#include "exathread.hpp"
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
	bool Resource::ValidateResourceAddr<Material>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::Material);
	}

	template<>
	bool Resource::ValidateResourceAddr<World>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::World);
	}

	template<>
	bool Resource::ValidateResourceAddr<BlobResource>(const std::string& addr) {
		return libcacaoasset::ValidateResourceAddress(addr, libcacaoasset::Resource::Type::Blob);
	}

	Resource::~Resource() {
		//Remove our pointer from the cache
		std::lock_guard lk {IMPL(ResourceManager).cacheProtector};
		IMPL(ResourceManager).cache.erase(address);
	}

	BlobResource::BlobResource(std::vector<unsigned char>&& data, const std::string& addr)
	  : Resource(addr), data(data) {
		Check<BadValueException>(ValidateResourceAddr<BlobResource>(addr), "Resource address is malformed!");
	}

	BlobResource::BlobResource(std::string&& data, const std::string& addr)
	  : Resource(addr), data(data.begin(), data.end()) {
		Check<BadValueException>(ValidateResourceAddr<BlobResource>(addr), "Resource address is malformed!");
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

	void ResourceManager::RegisterLoader(std::type_index tp, ResourceManager::ErasedLoader&& el) {
		impl->loaders[tp] = std::move(el);
	}
	exathread::ValueTask<std::shared_ptr<Resource>> ResourceManager::_AsyncLoadOpImpl(std::string address, std::type_index tp) {
		//Check cache
		std::shared_ptr<Resource> maybeCached = CheckCache(address);
		if(maybeCached) co_return maybeCached;

		//Embedded mesh/texture check
		if(tp == std::type_index(typeid(Mesh))) {
			Check<BadStateException>(IsLoaderRegistered(typeid(Model)), "No resource loader configured for the requested type!");

			//Split address string
			std::size_t slash = address.find_first_of('/');
			std::string model = address.substr(2, slash - 2);
			std::string mesh = address.substr(slash + 1);

			//First thing to check: do we have a ModelData instance already?
			if(impl->models.contains(model)) {
				//Great, just use that
				ResourceManager::Impl::ModelData& modelData = impl->models[model];
				modelData.loadedMeshes.insert(mesh);
				co_return modelData.mdl->GetMesh(mesh);
			} else {
				//Load the model
				ResourceManager::Impl::ModelData& modelData = impl->models[model];//[] syntax used to create
				exathread::Future<std::shared_ptr<Model>> modelFut = ResourceManager::Get().Load<Model>(std::format("a:{}", model));
				co_await exathread::yieldUntilComplete(modelFut);
				modelData.mdl = *modelFut;

				//Now return the mesh
				modelData.loadedMeshes.insert(mesh);
				co_return modelData.mdl->GetMesh(mesh);
			}

			//Check if the model data can be released due to all embedded assets being cached
			ResourceManager::Impl::ModelData& modelData = impl->models[model];
			if(modelData.loadedMeshes.size() == modelData.mdl->ListMeshes().size() && modelData.loadedTextures.size() == modelData.mdl->ListTextures().size()) {
				modelData.mdl.reset();
				impl->models.erase(model);
			}
		} else if(tp == std::type_index(typeid(Tex2D)) && address[0] == 'e') {
			Check<BadStateException>(IsLoaderRegistered(typeid(Model)), "No resource loader configured for the requested type!");

			//Split address string
			std::size_t slash = address.find_first_of('/');
			std::string model = address.substr(2, slash - 2);
			std::string tex = address.substr(slash + 1);

			//First thing to check: do we have a ModelData instance already?
			if(impl->models.contains(model)) {
				//Great, just use that
				ResourceManager::Impl::ModelData& modelData = impl->models[model];
				modelData.loadedTextures.insert(tex);
				co_return modelData.mdl->GetTexture(tex);
			} else {
				//Load the model
				ResourceManager::Impl::ModelData& modelData = impl->models[model];//[] syntax used to create
				exathread::Future<std::shared_ptr<Model>> modelFut = ResourceManager::Get().Load<Model>(std::format("a:{}", model));
				co_await exathread::yieldUntilComplete(modelFut);
				modelData.mdl = *modelFut;

				//Now return the mesh
				modelData.loadedTextures.insert(tex);
				co_return modelData.mdl->GetTexture(tex);
			}

			//Check if the model data can be released due to all embedded assets being cached
			ResourceManager::Impl::ModelData& modelData = impl->models[model];
			if(modelData.loadedMeshes.size() == modelData.mdl->ListMeshes().size() && modelData.loadedTextures.size() == modelData.mdl->ListTextures().size()) {
				modelData.mdl.reset();
				impl->models.erase(model);
			}
		}

		//Resource was not in cache, we need to load it
		//Check for a valid loader
		Check<BadStateException>(IsLoaderRegistered(tp), "No resource loader configured for the requested type!");

		//Try to load the asset
		co_return InvokeLoader(tp, address);
	}
}