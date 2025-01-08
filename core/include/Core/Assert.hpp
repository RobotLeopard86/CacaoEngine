#pragma once

#include <string>

#include "DllHelper.hpp"

namespace Cacao {
	/**
	 * @brief Check if the condition is true, and if not exit
	 * @warning If the condition is false, the engine will immediately shut down without performing regular shutdown routines (effectively crashing). AVOID IF NOT ABSOLUTELY NECESSARY.
	 *
	 * @param condition The condition to check
	 * @param errorMsg The message to log if the assertion fails
	 *
	 * @note For use only by the engine internally.
	 */
	CACAO_API void EngineAssert(bool condition, std::string errorMsg);

	/**
	 * @brief Check if the condition is true, and if not exit
	 * @warning If the condition is false, the engine will immediately shut down without performing regular shutdown routines (effectively crashing). AVOID IF NOT ABSOLUTELY NECESSARY.
	 *
	 * @param condition The condition to check
	 * @param errorMsg The message to log if the assertion fails
	 *
	 * @note For use only by Cacao Engine runtimes.
	 */
	CACAO_API void RuntimeAssert(bool condition, std::string errorMsg);

	/**
	 * @brief Check if the condition is true, and if not exit
	 * @warning If the condition is false, the engine will immediately shut down without performing regular shutdown routines (effectively crashing). AVOID IF NOT ABSOLUTELY NECESSARY.
	 *
	 * @param condition The condition to check
	 * @param errorMsg The message to log if the assertion fails
	 *
	 * @note For use by games using the engine.
	 */
	CACAO_API void ClientAssert(bool condition, std::string errorMsg);
}