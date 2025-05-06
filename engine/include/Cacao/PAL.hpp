#pragma once

#include "DllHelper.hpp"

#include <string>

namespace Cacao {
	/**
	 * @brief Platform abstraction layer for managing platform-specific behavior
	 *
	 * @warning This class is only for engine use. While it is documented, <b>no clients should use this class</b>
	 *
	 */
	class PAL {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static PAL& Get() {
			static PAL _instance;
			return _instance;
		}

		///@cond
		PAL(const PAL&) = delete;
		PAL(PAL&&) = delete;
		PAL& operator=(const PAL&) = delete;
		PAL& operator=(PAL&&) = delete;
		///@endcond

		/**
		 * @brief Set the active windowing module
		 *
		 * @param mod The name of the new module to use
		 *
		 * @throws NonexistentValueException If the requested module does not exist
		 * @throws BadValueException If the requested module is incompatible with the platform
		 * @throws MiscException If there are still PAL-backed objects using a different windowing module
		 */
		void SetWindowMod(const std::string& mod);

		/**
		 * @brief Set the active graphics module
		 *
		 * @param mod The name of the new module to use
		 *
		 * @throws NonexistentValueException If the requested module does not exist
		 * @throws BadValueException If the requested module is incompatible with the platform
		 * @throws MiscException If there are still PAL-backed objects using a different graphics module
		 */
		void SetGfxMod(const std::string& mod);

		/**
		 * @brief Configure the implementation pointer for a PAL-backed object
		 *
		 * Loads the implementation pointer with an interface object from the loaded windowing/graphics module
		 */
		template<typename T>
		void ConfigureImplPtr(T& obj) = delete;

	  private:
		PAL() {}
	};
}