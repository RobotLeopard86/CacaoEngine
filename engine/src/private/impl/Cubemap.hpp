#pragma once

#include "Cacao/Cubemap.hpp"

#include <optional>

namespace Cacao {
	class Cubemap::Impl {
	  public:
		virtual void Realize(bool& success) = 0;
		virtual void DropRealized() = 0;

		//Order: +X, -X, +Y, -Y, +Z, -Z
		std::array<libcacaoimage::Image, 6> faces;

		virtual ~Impl() = default;
	};
}