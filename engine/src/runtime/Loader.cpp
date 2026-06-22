#include "Bytestream.hpp"
#include "Cacao/Cubemap.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Mesh.hpp"
#include "Cacao/Model.hpp"
#include "Cacao/ResourceManager.hpp"
#include "Cacao/Shader.hpp"
#include "Cacao/Sound.hpp"
#include "Cacao/Tex2D.hpp"
#include "Cacao/World.hpp"

#include "libcacaoasset.hpp"

#include "Runtime.hpp"
#include "libcacaoimage.hpp"

#include <exception>
#include <fstream>
#include <string>
#include <memory>

class RTLoader {
  public:
	template<typename T>
	struct intermediate {};

	template<>
	struct intermediate<Cubemap> {
		using type = libcacaoasset::Cubemap;
	};
	template<>
	struct intermediate<Shader> {
		using type = libcacaoasset::Shader;
	};
	template<>
	struct intermediate<Tex2D> {
		using type = std::vector<unsigned char>;
	};
	template<>
	struct intermediate<Model> {
		using type = std::vector<unsigned char>;
	};
	template<>
	struct intermediate<Mesh> {
		using type = std::pair<std::vector<Vertex>, std::vector<glm::uvec3>>;
	};
	template<>
	struct intermediate<Sound> {
		using type = std::vector<char>;
	};
	template<>
	struct intermediate<World> {
		using type = libcacaoasset::World;
	};

	template<typename T>
	using intermediate_t = intermediate<T>::type;

	template<typename T>
	std::unique_ptr<intermediate_t<T>> FetchData(const std::string& addr) const;

	template<typename T>
	std::shared_ptr<T> CreateResource(const std::string& addr, std::unique_ptr<intermediate_t<T>>&& data) const;
};

void CfgLoader() {
	RTLoader l;
	ResourceManager::Get().ConfigureResourceLoader<RTLoader, World, Cubemap>(std::move(l));
}

template<>
std::unique_ptr<libcacaoasset::World> RTLoader::FetchData<World>(const std::string& addr) const {
	Check<NonexistentValueException>(rt.worldScan.contains(addr), "Cannot load a nonexistent world!");

	//Open file stream
	std::ifstream* ifs = new std::ifstream(rt.worldScan[addr]);
	Check<IOException>(ifs && ifs->is_open(), "Failed to open world file stream!");

	//Decode world and return (pointer will be freed by DecodeWorld)
	try {
		return std::make_unique<libcacaoasset::World>(libcacaoasset::DecodeWorld(ifs));
	} catch(const std::exception& e) {
		Check<ExternalException>(false, e.what());
		throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
	}
}

template<>
std::shared_ptr<World> RTLoader::CreateResource<World>(const std::string& addr, std::unique_ptr<libcacaoasset::World>&& data) const {
	return World::Create(*data, addr);
}

template<>
std::unique_ptr<libcacaoasset::Cubemap> RTLoader::FetchData<Cubemap>(const std::string& addr) const {
	Check<NonexistentValueException>(rt.resourceScan.contains(addr), "Cannot load a nonexistent cubemap!");

	//Open asset pack and get resource
	libcacaoasset::AssetPack pak = libcacaoasset::AssetPack::OpenFromFile(rt.resourceScan[addr]);
	libcacaoasset::Resource r = pak.GetResource(addr);
	Check<BadValueException>(r.type == libcacaoasset::Resource::Type::Cubemap, "Tried to load an asset as a cubemap that was not one!");

	//Decode world and return (pointer will be freed by DecodeCubemap)
	ibytestream* ibs = new ibytestream(r.bytes);
	libcacaoasset::Cubemap cmap;
	try {
		cmap = libcacaoasset::DecodeCubemap(ibs);
	} catch(const std::exception& e) {
		Check<ExternalException>(false, e.what());
		throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
	}
	return std::make_unique<libcacaoasset::Cubemap>(std::move(cmap));
}

template<>
std::shared_ptr<Cubemap> RTLoader::CreateResource<Cubemap>(const std::string& addr, std::unique_ptr<libcacaoasset::Cubemap>&& data) const {
	//Create faces
	std::array<libcacaoimage::Image, 6> faces;
	{
		ibytestream ibs {data->right};
		faces[0] = libcacaoimage::decode::DecodeGeneric(ibs);
	}
	{
		ibytestream ibs {data->left};
		faces[1] = libcacaoimage::decode::DecodeGeneric(ibs);
	}
	{
		ibytestream ibs {data->top};
		faces[2] = libcacaoimage::decode::DecodeGeneric(ibs);
	}
	{
		ibytestream ibs {data->bottom};
		faces[3] = libcacaoimage::decode::DecodeGeneric(ibs);
	}
	{
		ibytestream ibs {data->front};
		faces[4] = libcacaoimage::decode::DecodeGeneric(ibs);
	}
	{
		ibytestream ibs {data->back};
		faces[5] = libcacaoimage::decode::DecodeGeneric(ibs);
	}

	//Return cubemap asset
	return Cubemap::Create(std::move(faces), addr);
}