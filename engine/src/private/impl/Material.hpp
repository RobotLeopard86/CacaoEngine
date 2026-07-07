#pragma once

#include "Cacao/Material.hpp"

#include <unordered_map>

namespace Cacao {
	class Material::Impl {
	  public:
		virtual void Upload() = 0;

		std::shared_ptr<Shader> shader;
		std::unordered_map<std::string, ParamValue> storage;

		virtual ~Impl() = default;
	};
}