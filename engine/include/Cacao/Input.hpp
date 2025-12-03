#pragma once

#include "DllHelper.hpp"

#include "glm/glm.hpp"

#include <memory>

namespace Cacao {
	/**
	 * @brief Input state management singleton
	 */
	class CACAO_API Input {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static Input& Get();

		///@cond
		Input(const Input&) = delete;
		Input(Input&&) = delete;
		Input& operator=(const Input&) = delete;
		Input& operator=(Input&&) = delete;
		///@endcond

		/**
		 * @brief Get the current cursor position
		 *
		 * @return The current cursor position
		 */
		glm::dvec2 GetCursorPos();

		/**
		 * @brief Check whether a given key is pressed
		 *
		 * @param key The key to check.
		 * @note View the page "Input Mappings" in the manual for the list of valid keys
		 *
		 * @return If the key is pressed
		 */
		bool IsKeyPressed(unsigned int key);

		/**
		 * @brief Check whether a given mouse button is pressed
		 *
		 * @param button The mouse button to check.
		 * @note View the page "Input Mappings" in the manual for the list of valid buttons
		 *
		 * @return If the mouse button is pressed
		 */
		bool IsMouseButtonPressed(unsigned int button);

		///@cond
		struct Impl;
		///@endcond
	  private:
		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		Input();
		~Input();

		friend class TickController;

		void FreezeInputState();
	};
}