#pragma once

#include "Cacao/Script.hpp"

using namespace Cacao;

namespace sandbox {
	class ASTRA_REFLECT Rotator : public Cacao::Script {
	  public:
		Rotator() {}

		void OnDynTick(std::chrono::seconds timestep) override;

		ASTRASETUP(Rotator)
	};
}