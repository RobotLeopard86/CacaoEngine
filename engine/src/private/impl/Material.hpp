#pragma once

#include "Cacao/Material.hpp"

namespace Cacao {
	class Material::Impl {
	  public:
		virtual void Bake(bool& success) = 0;
		virtual void Discard() = 0;

		virtual ~Impl() = default;
	};
}