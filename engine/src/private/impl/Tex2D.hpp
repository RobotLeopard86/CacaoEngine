#pragma once

#include "Cacao/Tex2D.hpp"

#include <optional>

namespace Cacao {
	class Tex2D::Impl {
	  public:
		virtual void Realize(bool& success) = 0;
		virtual void DropRealized() = 0;

		libcacaoimage::Image img;

		virtual ~Impl() = default;
	};
}