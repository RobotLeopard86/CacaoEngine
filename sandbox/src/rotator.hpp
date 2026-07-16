#pragma once

#include "Cacao/Script.hpp"

namespace sandbox {
	class ASTRA_REFLECT Rotator : public Cacao::Script {
	  public:
		Rotator() {}

		void OnDynTick(std::chrono::seconds timestep) override;

		ASTRASETUP(Rotator)
	};
}