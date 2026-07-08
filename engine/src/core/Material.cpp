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
			//Find parameter object
			auto it = std::find_if(descriptor.uniformParams.cbegin(), descriptor.uniformParams.cend(), [&name](const libcacaoasset::Shader::Descriptor::UniformParameter& param) {
				return param.name.compare(name) == 0;
			});
			Check<NonexistentValueException>(it != descriptor.uniformParams.cend(), "Cannot set the value of a nonexistent material parameter!");

			//Type check
			switch(it->type) {
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int:
					Check<BadTypeException>(value.index() == 0, "Cannot upload non-int to int material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt:
					Check<BadTypeException>(value.index() == 1, "Cannot upload non-uint to uint material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float:
					Check<BadTypeException>(value.index() == 2, "Cannot upload non-float to float material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Bool:
					Check<BadTypeException>(value.index() == 3, "Cannot upload non-boolean to boolean material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int2:
					Check<BadTypeException>(value.index() == 4, "Cannot upload non-int2 to int2 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int3:
					Check<BadTypeException>(value.index() == 5, "Cannot upload non-int3 to int3 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int4:
					Check<BadTypeException>(value.index() == 6, "Cannot upload non-int4 to int4 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt2:
					Check<BadTypeException>(value.index() == 7, "Cannot upload non-uint2 to uint2 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt3:
					Check<BadTypeException>(value.index() == 8, "Cannot upload non-uint3 to uint3 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt4:
					Check<BadTypeException>(value.index() == 9, "Cannot upload non-uint4 to uint4 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float2:
					Check<BadTypeException>(value.index() == 10, "Cannot upload non-float2 to float2 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float3:
					Check<BadTypeException>(value.index() == 11, "Cannot upload non-float3 to float3 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float4:
					Check<BadTypeException>(value.index() == 12, "Cannot upload non-float4 to float4 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float2x2:
					Check<BadTypeException>(value.index() == 13, "Cannot upload non-float2x2 to float2x2 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float3x3:
					Check<BadTypeException>(value.index() == 14, "Cannot upload non-float3x3 to float3x3 material parameter!");
					break;
				case libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float4x4:
					Check<BadTypeException>(value.index() == 15, "Cannot upload non-float4x4 to float4x4 material parameter!");
					break;
				default:
					break;
			}

			//Store value
			impl->storage[name] = value;
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

	void Material::SetRenderMode(libcacaoasset::Material::RenderMode mode) {
		impl->renderMode = mode;
	}

	libcacaoasset::Material::RenderMode Material::GetRenderMode() {
		return impl->renderMode;
	}
}