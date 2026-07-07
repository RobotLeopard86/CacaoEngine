#pragma once

#include "Cacao/Shader.hpp"

namespace Cacao {
	class Shader::Impl {
	  public:
		virtual void Bake(bool& success) = 0;
		virtual void Discard() = 0;

		std::vector<unsigned char> irBuffer;
		libcacaoasset::Shader::Descriptor description;

		virtual ~Impl() = default;
	};
}