#pragma once

#include "DllHelper.hpp"

#include <string>
#include <memory>

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
		 * @brief Set the active module
		 *
		 * @param mod The name of the new module to use
		 *
		 * @throws NonexistentValueException If the requested module does not exist
		 * @throws BadValueException If the requested module is incompatible with the platform
		 * @throws MiscException If there are still PAL-backed objects using a different module
		 * @throws MiscException If the backend fails to load
		 */
		void SetModule(const std::string& mod);

		/**
		 * @brief Attempt to initialize the active module
		 *
		 * @throws NonexistentValueException If there is no active module
		 * @throws BadInitStateException If the module has already been initialized
		 *
		 * @return Whether initialization succeeds
		 */
		bool TryInitActiveModule();

		/**
		 * @brief Shutdown the active module and unload it
		 *
		 * @throws NonexistentValueException If there is no active module
		 */
		void Unload();

		/**
		 * @brief Configure the implementation pointer for a PAL-backed object
		 *
		 * Loads the implementation pointer with an interface object from the loaded module
		 *
		 * @throws MiscException If the module necessary to configure the pointer is loaded
		 */
		template<typename T>
		void ConfigureImplPtr(T& obj) = delete;

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		PAL();
		~PAL();
	};
}