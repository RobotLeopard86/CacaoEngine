#pragma once

#include "DllHelper.hpp"

#include <string>

namespace Cacao {
	/**
	 * @brief Package ordering handler singleton
	 */
	class CACAO_API OverlayStack {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static OverlayStack& Get();

		///@cond
		OverlayStack(const OverlayStack&) = delete;
		OverlayStack(OverlayStack&&) = delete;
		OverlayStack& operator=(const OverlayStack&) = delete;
		OverlayStack& operator=(OverlayStack&&) = delete;
		///@endcond

		/**
		 * @brief Register a new game package
		 *
		 * @param id The ID of the new package; should be in reverse domain format (e.g. com.example.MyPkg), but this is not enforced
		 *
		 * @throws ExistingValueException If there is already a package with that ID
		 */
		void RegisterPkg(const std::string& id);

		/**
		 * @brief Unregister a game package
		 *
		 * @param id The ID of the package
		 *
		 * @throws NonexistentValueException If there is already no package with that ID
		 * @throws MiscException If the requested package is still used in the stack
		 */
		void UnregisterPkg(const std::string& id);

	  private:
		OverlayStack();
	};
}