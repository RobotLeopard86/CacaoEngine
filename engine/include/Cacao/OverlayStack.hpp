#pragma once

#include "DllHelper.hpp"

#include "crossguid/guid.hpp"

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
		 * @brief Package description object used to identify the capabilities and contents of the package
		 *
		 * @todo This will be finalized later
		 */
		struct PackageDescriptor;

		/**
		 * @brief Register a new game package
		 *
		 * @param id The ID of the new package; should be in reverse domain format (e.g. com.example.MyPkg), but this is not enforced
		 * @param pkgDesc The package descriptor
		 *
		 * @throws ExistingValueException If there is already a package with that ID
		 */
		void RegisterPkg(const std::string& id, const PackageDescriptor& pkgDesc);

		/**
		 * @brief Unregister a game package
		 *
		 * @param id The ID of the package
		 *
		 * @throws NonexistentValueException If there is already no package with that ID
		 * @throws MiscException If the requested package is hosting a resource that is not provided by another package in the stack
		 */
		void UnregisterPkg(const std::string& id);

		/**
		 * @brief Set the position of a package in the stack
		 *
		 * @param id The ID of the package to move
		 * @param pos The new position of the package; this will be clamped to the stack size if exceeding it
		 *
		 * @throws NonexistentValueException If there is already no package with that ID
		 * @throws BadValueException If the package specified is the primary package
		 *
		 * @verbatim embed:rst
			.. tip::
				Because the ``pos`` parameter is clamped, you can use ``-INT8_MAX`` and ``INT8_MAX`` to move a package to the bottom and top of the stack respectively.
			@endverbatim
		 */
		void SetPkgPosition(const std::string& id, int8_t pos);

		/**
		 * @brief Set a package as the primary package
		 *
		 * @note This will effectively move the package to the bottom of the stack, since nothing can be below the primary package
		 *
		 * @param id The ID of the package to make the primary package
		 *
		 * @throws NonexistentValueException If there is already no package with that ID
		 */
		void SetPrimaryPkg(const std::string& id);

		/**
		 * @brief Convert a resource address to a resource cache lookup ID
		 *
		 * @param address The resource address to resolve
		 *
		 * @throws NonexistentValueException If there is no known package that can provide the requested resource
		 */
		xg::Guid ResolveResourceAddr(const std::string& address);

	  private:
		OverlayStack();
	};
}