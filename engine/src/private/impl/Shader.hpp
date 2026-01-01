#pragma once

#include "Cacao/Shader.hpp"

namespace Cacao {
	class Shader::Impl {
	  public:
		virtual void Realize(bool& success) = 0;
		virtual void DropRealized() = 0;

		std::vector<unsigned char> irBuffer;
		ShaderDescription description;

		virtual ~Impl() = default;
	};
}