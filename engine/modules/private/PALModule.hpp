#pragma once

#include "Cacao/DllHelper.hpp"

#include <functional>
#include <memory>

class PALInterface;

namespace Cacao {
	class CACAO_API PALModule {
	  public:
		enum class Type {
			Windowing,
			Graphics
		} const type;

		struct CACAO_API Factory {
			enum class Type {
				Window,
				Shader,
				Tex2D,
				Cubemap,
				Mesh
			} const type;
			const std::function<std::shared_ptr<PALInterface>()> func;
		};

		const std::function<std::vector<Factory>()> factories;

		const std::string id;
	};
}