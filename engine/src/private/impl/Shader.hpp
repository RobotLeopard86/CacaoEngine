#pragma once

#include "Cacao/Shader.hpp"

namespace Cacao {
	class Shader::Impl {
	  public:
		virtual void Bake(bool& success) = 0;
		virtual void Discard() = 0;

		//Shader info
		std::vector<unsigned char> irBuffer;
		libcacaoasset::Shader::Descriptor descriptor;

		//Custom shader compilation settings
		struct CustomCompileSettings {
			bool blendUseSrc;
			enum class Depth {
				Off,
				Less,
				Lequal
			} depth;
		};
		std::optional<CustomCompileSettings> customSettings;

		virtual ~Impl() = default;
	};
}