#pragma once

#include "Core/DllHelper.hpp"

namespace Cacao {
	/**
	 * @brief Engine configuration values
	 */
	struct CACAO_API EngineConfig {
		/**
		 * @brief The number of dynamic ticks that should occur within a second
		 * @details This is NOT a hard constraint. Attempting to run too many ticks within a second will cause noticeable frame skipping.
		 */
		int targetDynTPS;

		/**
		 * @brief The rate at which fixed ticks should occur
		 *
		 * @warning Fixed ticks are not yet implemented into the engine!
		 */
		int fixedTickRate;

		///@brief The number of frames the renderer can be behind before skipping some to catch up
		int maxFrameLag;
	};
}