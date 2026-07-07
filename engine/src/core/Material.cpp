#include "Cacao/Material.hpp"
#include "Cacao/PAL.hpp"
#include "Cacao/Exceptions.hpp"
#include "impl/Material.hpp"
#include "impl/ResourceManager.hpp"
#include "ImplAccessor.hpp"
#include "PALConfigurables.hpp"

#include "libcacaoasset.hpp"

#include <variant>

namespace Cacao {
	Material::Material(std::shared_ptr<Shader> shader, const std::string& addr)
	  : Resource(addr) {
		Check<BadValueException>(ValidateResourceAddr<Material>(addr), "Resource address is malformed!");
		Check<Shader, BadValueException>(shader, "Cannot construct a material with a null shader handle!");

		//Create implementation pointer
		PAL::Get().ConfigureImplPtr(*this);

		//Fill data
		impl->shader = shader;
	}

	std::shared_ptr<Material> Material::Create(std::shared_ptr<Shader> shader, const std::string& addr) {
		std::shared_ptr<Material> ptr(new Material(shader, addr));
		IMPL(ResourceManager).cache.insert_or_assign(addr, ptr);
		return ptr;
	}

	Material::Material(Material&& other)
	  : Resource(other.address) {
		//Steal the implementation pointer
		impl = std::move(other.impl);

		//Blank out other asset address
		other.address = "";
	}

	Material& Material::operator=(Material&& other) {
		//Implementation pointer
		impl = std::move(other.impl);

		//Asset address
		address = other.address;
		other.address = "";

		return *this;
	}

	void Material::_SetParam(const std::string& name, const ParamValue& value) {
		//Get descriptor
		libcacaoasset::Shader::Descriptor descriptor = impl->shader->GetDescriptor();

		//Check if a texture was passed for searching
		//The textures are at the end of ParamValue so this will check for that
		if(value.index() >= std::variant_size_v<ParamValue> - 2) {
			//Find parameter object
			auto it = std::find_if(descriptor.texParams.cbegin(), descriptor.texParams.cend(), [&name](const libcacaoasset::Shader::Descriptor::TextureParameter& param) {
				return param.name.compare(name) == 0;
			});
			Check<NonexistentValueException>(it != descriptor.texParams.cend(), "Cannot set the value of a nonexistent material parameter!");

			//Handle check
			if(it->isCubemap) {
				Check<BadTypeException>(value.index() == std::variant_size_v<ParamValue> - 1, "Cannot set the value of a material parameter with a non-matching type!");
				Check<Cubemap, NonexistentValueException>(std::get<std::shared_ptr<Cubemap>>(value), "Cannot set a material parameter to an empty handle!");
			} else {
				Check<BadTypeException>(value.index() == std::variant_size_v<ParamValue> - 2, "Cannot set the value of a material parameter with a non-matching type!");
				Check<Tex2D, NonexistentValueException>(std::get<std::shared_ptr<Tex2D>>(value), "Cannot set a material parameter to an empty handle!");
			}

			//Store value
			impl->storage[name] = value;
		} else {
		}
	}

	Material::ParamValue Material::_GetParam(const std::string& name) {
		//Storage check
		Check<NonexistentValueException>(impl->storage.contains(name), "Cannot get the value of an unset material parameter!");

		//Check paraneter existence
		libcacaoasset::Shader::Descriptor descriptor = impl->shader->GetDescriptor();
		auto texIt = std::find_if(descriptor.texParams.cbegin(), descriptor.texParams.cend(), [&name](const libcacaoasset::Shader::Descriptor::TextureParameter& param) {
			return param.name.compare(name) == 0;
		});
		auto ufmIt = std::find_if(descriptor.uniformParams.cbegin(), descriptor.uniformParams.cend(), [&name](const libcacaoasset::Shader::Descriptor::UniformParameter& param) {
			return param.name.compare(name) == 0;
		});
		Check<NonexistentValueException>(ufmIt != descriptor.uniformParams.cend() || texIt != descriptor.texParams.cend(), "Cannot get the value of a nonexistent material parameter!");

		//Return result
		return impl->storage[name];
	}
}