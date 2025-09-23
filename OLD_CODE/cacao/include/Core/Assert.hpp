#pragma once

#include <string>

namespace Cacao {
	/**
	 * @brief Check if the condition is true, and if not exit
	 * @warning If the condition is false, the engine will immediately shut down without calling anything else. Prefer CheckException when possible.
	 *
	 * @param condition The condition to check
	 * @param errorMsg The message to log if the assertion fails
	 *
	 * @note For use only by the engine internally.
	 */
	void EngineAssert(bool condition, std::string errorMsg);

	/**
	 * @brief Check if the condition is true, and if not exit
	 * @warning If the condition is false, the engine will immediately shut down without calling anything else. Prefer CheckException when possible.
	 *
	 * @param condition The condition to check
	 * @param errorMsg The message to log if the assertion fails
	 *
	 * @note For use by games using the engine.
	 */
	void ClientAssert(bool condition, std::string errorMsg);
}