#pragma once

#include "Cacao/Tex2D.hpp"

namespace Cacao {
	class Tex2D::Impl {
	  public:
		virtual void Bake(bool& success) = 0;
		virtual void Discard() = 0;

		libcacaoimage::Image img;

		virtual ~Impl() = default;
	};
}