#pragma once

#include <string>
#include <utility>
#include <map>

#include "glm/vec2.hpp"

#include "Utilities/MiscUtils.hpp"
#include "Core/Engine.hpp"

namespace Cacao {
	/**
	 * @brief The different window display modes
	 */
	enum class WindowMode {
		Window,	   ///<Normal window
		Fullscreen,///<Exclusive fullscreen (only on primary monitor)
		Borderless ///<Borderless fullscreen window (only on primary monitor)
	};

	/**
	 * @brief Window singleton
	 */
	class Window {
	  public:
		/**
		 * @brief Open the window
		 *
		 * @param title The window title
		 * @param initialSize The initial size of the window
		 * @param startVisible If the window should open visible
		 * @param mode The intial mode the window should be in
		 *
		 * @note For use by the engine only and automatically called by it at startup
		 *
		 * @throws Exception If the window is open already
		 */
		void Open(std::string title, glm::uvec2 initialSize, bool startVisible, WindowMode mode);

		/**
		 * @brief Close the window
		 *
		 * @note For use by the engine only and automatically called by it at shutdown
		 *
		 * @throws Exception If the window isn't open
		 */
		void Close();

		/**
		 * @brief Check if the window is open or not
		 *
		 * @return Whether the window is open or not
		 */
		bool IsOpen() {
			return isOpen;
		}

		/**
		 * @brief Query the OS for events and process them
		 *
		 * @throws Exception If the window isn't open
		 */
		void Update();

		/**
		 * @brief Present the last drawn frame to the window
		 *
		 * @throws Exception If the window isn't open
		 */
		void Present();

		/**
		 * @brief Get the window size
		 *
		 * @return The window size, or {0, 0} if the window isn't open
		 */
		glm::uvec2 GetSize() {
			if(!isOpen) return glm::uvec2 {0};
			return size;
		}

		/**
		 * @brief Set the window size
		 *
		 * @throws Exception If the window isn't open
		 */
		void SetSize(glm::uvec2 newSize) {
			CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't set the size of an unopened window!");
			size = newSize;
			UpdateWindowSize();
		}

		/**
		 * @brief Get the size of the content area
		 * @details The content area is the area where the game is drawn, excluding window decorations
		 *
		 * @return The content area size in pixels
		 */
		glm::uvec2 GetContentAreaSize();

		/**
		 * @brief Enable or disable VSync
		 *
		 * @param value The new VSync state
		 *
		 * @throws Exception If the window isn't open
		 */
		void SetVSyncEnabled(bool value) {
			CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't set the size of an unopened window!");
			useVSync = value;
			UpdateVSyncState();
		}

		/**
		 * @brief Check if VSync is enabled
		 *
		 * @return If VSync is enabled or simply false if the window isn't open
		 */
		bool IsVSyncEnabled() {
			if(!isOpen) return false;
			return useVSync;
		}

		/**
		 * @brief Set the window title
		 *
		 * @param title The new title
		 *
		 * @throws Exception If the window isn't open
		 */
		void SetTitle(std::string title);

		/**
		 * @brief Get the window title
		 *
		 * @return The window title or an empty string if the window isn't open
		 */
		std::string GetWindowTitle() {
			if(!isOpen) return "";
			return windowTitle;
		}

		/**
		 * @brief Show/hide the window
		 *
		 * @param value The new visibility state
		 *
		 * @throws Exception If the window isn't open
		 */
		void SetWindowVisibility(bool value) {
			CheckException(isOpen, Exception::GetExceptionCodeFromMeaning("BadState"), "Can't set the size of an unopened window!");
			isVisible = value;
			UpdateVisibilityState();
		}

		/**
		 * @brief Check if the window is visible
		 *
		 * @return If the window is visible or simply false if the window isn't open
		 */
		bool IsWindowVisible() {
			if(!isOpen) return false;
			return isVisible;
		}

		/**
		 * @brief Set the window mode
		 *
		 * @param newMode The new mode
		 *
		 * @throws Exception If the window isn't open
		 */
		void SetMode(WindowMode newMode) {
			if(!isOpen) return;
			WindowMode last = mode;
			mode = newMode;
			UpdateModeState(last);
		}

		/**
		 * @brief Get the current window mode
		 *
		 * @return The current window mode or WindowMode::Window if the window isn't open
		 */
		WindowMode GetMode() {
			if(!isOpen) return WindowMode::Window;
			return mode;
		}

		/**
		 * @brief Check if the window is minimized/iconified
		 *
		 * @return If the window is minimized or true if the window isn't open
		 */
		bool IsMinimized() {
			return (!isOpen) || minimized;
		}

		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static Window* GetInstance();

	  private:
		Window()
		  : isOpen(false), mode(WindowMode::Window), minimized(false) {}

		//Singleton data
		static Window* instance;
		static bool instanceExists;

		bool isOpen;
		bool isVisible;

		bool useVSync;
		glm::uvec2 size;
		std::string windowTitle;

		WindowMode mode;
		bool minimized;

		//The last known window position
		//Used for switching between modes
		glm::ivec2 windowedPosition;

		void UpdateVSyncState();
		void UpdateWindowSize();
		void UpdateVisibilityState();
		void UpdateModeState(WindowMode lastMode);

		//For changing the window size from implementation without generating more resize events
		friend void ChangeSize(Window* win, glm::uvec2 size) {
			win->size = size;
		}
		friend void NotifyMinimizeState(Window* win, bool newState) {
			win->minimized = newState;
		}
		friend struct WindowResizer;

		//Backend-implemented data type
		struct WindowData;

		std::shared_ptr<WindowData> nativeData;

		friend class Text;
	};
}