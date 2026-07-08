#pragma once

#include "Cacao/GPU.hpp"
#include "Cacao/Material.hpp"

#include <unordered_map>

namespace Cacao {
	class Material::Impl {
	  public:
		virtual void Upload(std::unique_ptr<CommandBuffer>& cmd) = 0;

		std::shared_ptr<Shader> shader;
		std::unordered_map<std::string, ParamValue> storage;
		libcacaoasset::Material::RenderMode renderMode;

		virtual ~Impl() = default;
	};
}