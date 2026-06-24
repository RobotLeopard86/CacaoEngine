#pragma once

#include "Cacao/Material.hpp"

namespace Cacao {
	class Material::Impl {
	  public:
		virtual void Realize(bool& success) = 0;
		virtual void DropRealized() = 0;

		virtual ~Impl() = default;
	};
}