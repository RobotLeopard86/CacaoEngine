#pragma once

#include "Cacao/Script.hpp"

using namespace Cacao;

namespace sandbox {
	class ASTRA_REFLECT SelfSpinner : public Cacao::Script {
	  public:
		SelfSpinner() {}

		void OnDynTick(float timestep) override;

		ASTRASETUP(SelfSpinner);
	};
}