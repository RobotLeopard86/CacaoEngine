#include "Bytestream.hpp"
#include "Cacao/Cubemap.hpp"
#include "Cacao/Engine.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/Material.hpp"
#include "Cacao/Mesh.hpp"
#include "Cacao/Model.hpp"
#include "Cacao/Resource.hpp"
#include "Cacao/ResourceManager.hpp"
#include "Cacao/Shader.hpp"
#include "Cacao/Sound.hpp"
#include "Cacao/Tex2D.hpp"
#include "Cacao/World.hpp"

#include "libcacaoasset.hpp"

#include "Runtime.hpp"
#include "libcacaoimage.hpp"

#include <cstring>
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
	struct intermediate<Sound> {
		using type = std::vector<char>;
	};
	template<>
	struct intermediate<World> {
		using type = libcacaoasset::World;
	};
	template<>
	struct intermediate<Material> {
		using type = libcacaoasset::Material;
	};
	template<>
	struct intermediate<BlobResource> {
		using type = std::vector<unsigned char>;
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
	ResourceManager::Get().ConfigureResourceLoader<RTLoader, World, Cubemap, Shader, Tex2D, Sound, Model, Material, BlobResource>(std::move(l));
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
	Check<BadValueException>(r.type == libcacaoasset::Resource::Type::Cubemap, "Tried to load a resource as a cubemap that is not one!");

	//Decode cubemap and return (pointer will be freed by DecodeCubemap)
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
		faces[0] = libcacaoimage::decode::DecodeWebP(ibs);
	}
	{
		ibytestream ibs {data->left};
		faces[1] = libcacaoimage::decode::DecodeWebP(ibs);
	}
	{
		ibytestream ibs {data->top};
		faces[2] = libcacaoimage::decode::DecodeWebP(ibs);
	}
	{
		ibytestream ibs {data->bottom};
		faces[3] = libcacaoimage::decode::DecodeWebP(ibs);
	}
	{
		ibytestream ibs {data->front};
		faces[4] = libcacaoimage::decode::DecodeWebP(ibs);
	}
	{
		ibytestream ibs {data->back};
		faces[5] = libcacaoimage::decode::DecodeWebP(ibs);
	}

	//Return cubemap asset
	std::shared_ptr<Cubemap> cmap = Cubemap::Create(std::move(faces), addr);
	cmap->Bake();
	return cmap;
}

template<>
std::unique_ptr<libcacaoasset::Shader> RTLoader::FetchData<Shader>(const std::string& addr) const {
	Check<NonexistentValueException>(rt.resourceScan.contains(addr), "Cannot load a nonexistent shader!");

	//Open asset pack and get resource
	libcacaoasset::AssetPack pak = libcacaoasset::AssetPack::OpenFromFile(rt.resourceScan[addr]);
	libcacaoasset::Resource r = pak.GetResource(addr);
	Check<BadValueException>(r.type == libcacaoasset::Resource::Type::Shader, "Tried to load a resource as a shader that is not one!");

	//Decode shader and return (pointer will be freed by DecodeShader)
	ibytestream* ibs = new ibytestream(r.bytes);
	libcacaoasset::Shader shader;
	try {
		shader = libcacaoasset::DecodeShader(ibs);
	} catch(const std::exception& e) {
		Check<ExternalException>(false, e.what());
		throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
	}
	return std::make_unique<libcacaoasset::Shader>(std::move(shader));
}

template<>
std::shared_ptr<Shader> RTLoader::CreateResource<Shader>(const std::string& addr, std::unique_ptr<libcacaoasset::Shader>&& data) const {
	std::shared_ptr<Shader> shader = Shader::Create(std::move(data->irCode), data->descriptor, addr);
	shader->Bake();
	return shader;
}

template<>
std::unique_ptr<std::vector<unsigned char>> RTLoader::FetchData<Tex2D>(const std::string& addr) const {
	Check<NonexistentValueException>(rt.resourceScan.contains(addr), "Cannot load a nonexistent 2D texture!");

	//Open asset pack and get resource
	libcacaoasset::AssetPack pak = libcacaoasset::AssetPack::OpenFromFile(rt.resourceScan[addr]);
	libcacaoasset::Resource r = pak.GetResource(addr);
	Check<BadValueException>(r.type == libcacaoasset::Resource::Type::Tex2D, "Tried to load a resource as a 2D texture that is not one!");

	return std::make_unique<std::vector<unsigned char>>(std::move(r.bytes));
}

template<>
std::shared_ptr<Tex2D> RTLoader::CreateResource<Tex2D>(const std::string& addr, std::unique_ptr<std::vector<unsigned char>>&& data) const {
	ibytestream ibs {*data};
	libcacaoimage::Image img = libcacaoimage::decode::DecodeGeneric(ibs);
	std::shared_ptr<Tex2D> tex = Tex2D::Create(std::move(img), addr);
	tex->Bake();
	return tex;
}

template<>
std::unique_ptr<std::vector<char>> RTLoader::FetchData<Sound>(const std::string& addr) const {
	Check<NonexistentValueException>(rt.resourceScan.contains(addr), "Cannot load a nonexistent sound!");

	//Open asset pack and get resource
	libcacaoasset::AssetPack pak = libcacaoasset::AssetPack::OpenFromFile(rt.resourceScan[addr]);
	libcacaoasset::Resource r = pak.GetResource(addr);
	Check<BadValueException>(r.type == libcacaoasset::Resource::Type::Audio, "Tried to load a resource as a sound that is not one!");

	//Convert to vector<char>
	std::unique_ptr<std::vector<char>> asChar = std::make_unique<std::vector<char>>();
	asChar->resize(r.bytes.size());
	std::memcpy(asChar->data(), r.bytes.data(), r.bytes.size());
	return asChar;
}

template<>
std::shared_ptr<Sound> RTLoader::CreateResource<Sound>(const std::string& addr, std::unique_ptr<std::vector<char>>&& data) const {
	std::shared_ptr<Sound> sound = Sound::Create(std::move(*data), addr);
	sound->Bake();
	return sound;
}

template<>
std::unique_ptr<std::vector<unsigned char>> RTLoader::FetchData<Model>(const std::string& addr) const {
	Check<NonexistentValueException>(rt.resourceScan.contains(addr), "Cannot load a nonexistent model!");

	//Open asset pack and get resource
	libcacaoasset::AssetPack pak = libcacaoasset::AssetPack::OpenFromFile(rt.resourceScan[addr]);
	libcacaoasset::Resource r = pak.GetResource(addr);
	Check<BadValueException>(r.type == libcacaoasset::Resource::Type::Model, "Tried to load a resource as a model that is not one!");

	return std::make_unique<std::vector<unsigned char>>(std::move(r.bytes));
}

template<>
std::shared_ptr<Model> RTLoader::CreateResource<Model>(const std::string& addr, std::unique_ptr<std::vector<unsigned char>>&& data) const {
	return Model::Create(std::move(*data), addr);
}

template<>
std::unique_ptr<std::vector<unsigned char>> RTLoader::FetchData<BlobResource>(const std::string& addr) const {
	Check<NonexistentValueException>(rt.resourceScan.contains(addr), "Cannot load a nonexistent blob resource!");

	//Open asset pack and get resource
	libcacaoasset::AssetPack pak = libcacaoasset::AssetPack::OpenFromFile(rt.resourceScan[addr]);
	libcacaoasset::Resource r = pak.GetResource(addr);
	Check<BadValueException>(r.type == libcacaoasset::Resource::Type::Blob, "Tried to load a resource as a blob that is not one!");

	return std::make_unique<std::vector<unsigned char>>(std::move(r.bytes));
}

template<>
std::shared_ptr<BlobResource> RTLoader::CreateResource<BlobResource>(const std::string& addr, std::unique_ptr<std::vector<unsigned char>>&& data) const {
	return BlobResource::Create(std::move(*data), addr);
}

template<>
std::unique_ptr<libcacaoasset::Material> RTLoader::FetchData<Material>(const std::string& addr) const {
	Check<NonexistentValueException>(rt.resourceScan.contains(addr), "Cannot load a nonexistent material!");

	//Open asset pack and get resource
	libcacaoasset::AssetPack pak = libcacaoasset::AssetPack::OpenFromFile(rt.resourceScan[addr]);
	libcacaoasset::Resource r = pak.GetResource(addr);
	Check<BadValueException>(r.type == libcacaoasset::Resource::Type::Material, "Tried to load a resource as a material that is not one!");

	//Decode material and return (pointer will be freed by DecodeMaterial)
	ibytestream* ibs = new ibytestream(r.bytes);
	libcacaoasset::Material material;
	try {
		material = libcacaoasset::DecodeMaterial(ibs);
	} catch(const std::exception& e) {
		Check<ExternalException>(false, e.what());
		throw std::runtime_error("UNREACHABLE CODE!!! HOW DID YOU GET HERE?!");//This will never be reached because of the Check call, but the compiler doesn't know what Check does, so we have to spell it out like it's 3
	}
	return std::make_unique<libcacaoasset::Material>(std::move(material));
}

template<>
std::shared_ptr<Material> RTLoader::CreateResource<Material>(const std::string& addr, std::unique_ptr<libcacaoasset::Material>&& data) const {
	std::shared_ptr<Material> m = Material::Create(*ResourceManager::Get().Load<Shader>(data->shaderAddress), addr);
	m->SetRenderMode(data->renderMode);
	for(const libcacaoasset::Material::Param& param : data->parameters) {
		switch(param.storage.index()) {
			case 0:
				m->SetParameter(param.target, std::get<int>(param.storage));
				break;
			case 1:
				m->SetParameter(param.target, std::get<unsigned int>(param.storage));
				break;
			case 2:
				m->SetParameter(param.target, std::get<float>(param.storage));
				break;
			case 3:
				m->SetParameter(param.target, std::get<bool>(param.storage));
				break;
			case 4: {
				libcacaoasset::Material::TexRef val = std::get<libcacaoasset::Material::TexRef>(param.storage);
				if(!val.isCubemap) {
					m->SetParameter(param.target, *ResourceManager::Get().Load<Tex2D>(val.address));
				} else {
					m->SetParameter(param.target, *ResourceManager::Get().Load<Cubemap>(val.address));
				}
			}
			case 5: {
				auto val = std::get<libjaguar::Vector<float, 2>>(param.storage);
				m->SetParameter(param.target, glm::vec2 {val.x, val.y});
				break;
			}
			case 6: {
				auto val = std::get<libjaguar::Vector<float, 3>>(param.storage);
				m->SetParameter(param.target, glm::vec3 {val.x, val.y, val.z});
				break;
			}
			case 7: {
				auto val = std::get<libjaguar::Vector<float, 4>>(param.storage);
				m->SetParameter(param.target, glm::vec4 {val.x, val.y, val.z, val.w});
				break;
			}
			case 8: {
				auto val = std::get<libjaguar::Vector<int, 2>>(param.storage);
				m->SetParameter(param.target, glm::ivec2 {val.x, val.y});
				break;
			}
			case 9: {
				auto val = std::get<libjaguar::Vector<int, 3>>(param.storage);
				m->SetParameter(param.target, glm::ivec3 {val.x, val.y, val.z});
				break;
			}
			case 10: {
				auto val = std::get<libjaguar::Vector<int, 4>>(param.storage);
				m->SetParameter(param.target, glm::ivec4 {val.x, val.y, val.z, val.w});
				break;
			}
			case 11: {
				auto val = std::get<libjaguar::Vector<unsigned int, 2>>(param.storage);
				m->SetParameter(param.target, glm::uvec2 {val.x, val.y});
				break;
			}
			case 12: {
				auto val = std::get<libjaguar::Vector<unsigned int, 3>>(param.storage);
				m->SetParameter(param.target, glm::uvec3 {val.x, val.y, val.z});
				break;
			}
			case 13: {
				auto val = std::get<libjaguar::Vector<unsigned int, 4>>(param.storage);
				m->SetParameter(param.target, glm::uvec4 {val.x, val.y, val.z, val.w});
				break;
			}
			case 14: {
				auto val = std::get<libjaguar::Matrix<float, 2, 2>>(param.storage);
				m->SetParameter(param.target, glm::mat2 {val[0][0], val[0][1], val[1][0], val[1][1]});
				break;
			}
			case 15: {
				auto val = std::get<libjaguar::Matrix<float, 3, 3>>(param.storage);
				m->SetParameter(param.target, glm::mat3 {val[0][0], val[0][1], val[0][2], val[1][0], val[1][1], val[1][2], val[2][0], val[2][1], val[2][2]});
				break;
			}
			case 16: {
				auto val = std::get<libjaguar::Matrix<float, 4, 4>>(param.storage);
				m->SetParameter(param.target, glm::mat4 {val[0][0], val[0][1], val[0][2], val[0][3], val[1][0], val[1][1], val[1][2], val[1][3], val[2][0], val[2][1], val[2][2], val[2][3], val[3][0], val[3][1], val[3][2], val[3][3]});
				break;
			}
		}
	}
	return m;
}