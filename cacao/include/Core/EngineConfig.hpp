#pragma once

namespace Cacao {
	//A struct representing engine configuration
	struct EngineConfig {
		//Target number of dynamic ticks per second
		int targetDynTPS;

		//How many milliseconds should elapse between fixed ticks
		int fixedTickRate;
	};
}