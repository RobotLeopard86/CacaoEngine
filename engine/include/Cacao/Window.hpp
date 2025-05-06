#pragma once

#include "DllHelper.hpp"

#include <memory>
#include <string>

#include "glm/vec2.hpp"

namespace Cacao {
	/**
	 * @brief Game window singleton
	 */
	class CACAO_API Window {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static Window& Get() {
			static Window _instance;
			return _instance;
		}

		///@cond
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;
		///@endcond

		/**
		 * @brief Window mode
		 */
		enum class Mode {
			Windowed,  ///<Standard window on the desktop
			Borderless,///<Window covering the whole desktop with no decorations
			Fullscreen ///<Fullscreen window with exclusive monitor control
		};

		//======================= WINDOW CONTROL =======================

		/**
		 * @brief Open the window
		 *
		 * @param title Initial window title
		 * @param size Initial window size
		 * @param visible Initial window visibility
		 * @param mode Initial window mode
		 *
		 * @throws BadInitStateException If the window is already open
		 * @throws BadValueException If the title string is empty
		 * @throws BadValueException If one of the size components is zero
		 * @throws BadValueException If the window is to be invisible and not in Windowed mode
		 */
		void Open(const std::string& title, glm::uvec2 size, bool visible, Mode mode);

		/**
		 * @brief Close the window
		 *
		 * @throws BadInitStateException If the window is not open
		 */
		void Close();

		//======================= PROPERTY SETTERS =======================

		/**
		 * @brief Make the window visible
		 *
		 * @throws BadInitStateException If the window is not open
		 * @throws BadStateException If the window is already visible
		 */
		void Show();

		/**
		 * @brief Hide the window
		 *
		 * @throws BadInitStateException If the window is not open
		 * @throws BadStateException If the window is already hidden
		 */
		void Hide();

		/**
		 * @brief Change the window title
		 *
		 * @param newTitle The new window title
		 *
		 * @throws BadInitStateException If the window is not open
		 * @throws BadValueException If the title string is empty
		 */
		void SetTitle(const std::string& newTitle);

		/**
		 * @brief Change the window size
		 *
		 * @param newSize The new window size
		 *
		 * @throws BadInitStateException If the window is not open
		 * @throws BadValueException If one of the size components is zero
		 */
		void Resize(const glm::uvec2& newSize);

		/**
		 * @brief Change the window mode
		 *
		 * @param newMode The new window mode
		 *
		 * @throws BadInitStateException If the window is not open
		 * @throws BadValueException If the window is invisible and to be moved from Windowed mode
		 */
		void SetMode(Mode newMode);

		//======================= PROPERTY GETTERS =======================

		/**
		 * @brief Check if the window is open
		 *
		 * @return Whether the window is open
		 */
		bool IsOpen() {
			return open;
		}

		/**
		 * @brief Check if the window is visible
		 *
		 * @return Whether the window is visible, or false if the window is not open
		 */
		bool IsVisible() {
			return open && visible;
		}

		/**
		 * @brief Check the window title
		 *
		 * @return The window title, or an empty string if the window is not open
		 */
		const std::string GetTitle() {
			return (open ? title : "");
		}

		/**
		 * @brief Check the window size
		 *
		 * @return The window size, or {0, 0} if the window is not open
		 */
		const glm::uvec2 GetSize() {
			return (open ? size : glm::uvec2 {0, 0});
		}

		/**
		 * @brief Check the pixel size of the content area
		 *
		 * @return The content area size, or {0, 0} if the window is not open
		 */
		const glm::uvec2 GetContentAreaSize();

		/**
		 * @brief Check the window mode
		 *
		 * @return The window mode, or Mode::Windowed if the window is not open
		 */
		const Mode GetMode() {
			return (open ? mode : Mode::Windowed);
		}

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;
		friend class PAL;
		friend class PALWindowInterface;

		Window();
		~Window();

		bool open, visible;
		Mode mode;
		glm::uvec2 size;
		std::string title;

		//This is only used for restoring window position when switching modes
		glm::ivec2 lastPos;
	};
}