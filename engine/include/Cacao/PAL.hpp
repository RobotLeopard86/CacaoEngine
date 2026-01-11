#pragma once

#include "DllHelper.hpp"

#include <string>
#include <memory>

namespace Cacao {
	/**
	 * @brief Platform abstraction layer for managing platform-specific behavior
	 *
	 * @warning This class is only for engine use. While it is documented, <b>no clients should use this class</b>
	 */
	class CACAO_API PAL {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static PAL& Get();

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
		bool InitializeModule();

		/**
		 * @brief Connect the window and graphics backend
		 *
		 * @throws NonexistentValueException If there is no active module
		 * @throws BadInitStateException If the module has not been initialized
		 * @throws BadStateException If the graphics backend and window are already connected
		 */
		void GfxConnect();

		/**
		 * @brief Disonnect the window and graphics backend
		 *
		 * @throws BadStateException If the graphics backend and window are not connected
		 */
		void GfxDisconnect();

		/**
		 * @brief Shutdown the active module
		 *
		 * @throws BadInitStateException If the module has not been initialized
		 * @throws BadStateException If the graphics backend and window are still connected
		 */
		void TerminateModule();

		/**
		 * @brief Configure the implementation pointer for a PAL-backed object
		 *
		 * Loads the implementation pointer with an interface object from the active module
		 *
		 * @throws BadInitStateException If the module has not been initialized
		 * @throws BadStateException If the graphics backend and window are not connected
		 */
		template<typename T>
		void ConfigureImplPtr(T& obj) = delete;

		///@cond
		struct Impl;
		///@endcond
	  private:
		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		PAL();
		~PAL();
	};
}