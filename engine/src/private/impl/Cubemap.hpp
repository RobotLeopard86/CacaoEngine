#pragma once

#include "Cacao/Cubemap.hpp"

#include <optional>

namespace Cacao {
	class Cubemap::Impl {
	  public:
		virtual void Bake(bool& success) = 0;
		virtual void Discard() = 0;

		//Order: +X, -X, +Y, -Y, +Z, -Z
		std::array<libcacaoimage::Image, 6> faces;

		virtual ~Impl() = default;
	};
}