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
		 */
		void Open(const std::string& title, glm::uvec2 size, bool visible, Mode mode);

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
		 * @brief Check if the window is minimized
		 *
		 * @return Whether the window is minimized, or false if the window is not open
		 */
		bool IsMinimized() {
			return open && minimized;
		}

		/**
		 * @brief Check if V-Sync is enabled
		 *
		 * @return Whether V-Sync is enabled, or false if the window is not open
		 */
		bool IsVSyncEnabled() {
			return open && vsync;
		}

		/**
		 * @brief Check the window title
		 *
		 * @return The window title, or an empty string if the window is not open
		 */
		const std::string& GetTitle() {
			return (open ? title : "");
		}

		/**
		 * @brief Check the window size
		 *
		 * @return The window size, or {0, 0} if the window is not open
		 */
		const glm::uvec2& GetSize() {
			return (open ? size : glm::uvec2 {0, 0});
		}

		/**
		 * @brief Check the window mode
		 *
		 * @return The window mode, or Mode::Windowed if the window is not open
		 */
		const Mode& GetMode() {
			return (open ? mode : Mode::Windowed);
		}

	  private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		Window();
		~Window();

		bool open, visible, vsync, minimized;
		Mode mode;
		glm::uvec2 size;
		std::string title;

		//This is only used for restoring window position when switching modes
		glm::ivec2 lastPos;
	};
}