#pragma once

#include <string>
#include <utility>
#include <map>

#include "glm/vec2.hpp"

#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	//Represents the three different window mode states
	//Fullscreen and borderless modes apply to only the primary monitor
	enum class WindowMode {
		Window,	   //Runs in a standard window
		Fullscreen,//Runs in exclusive fullscreen mode, controlling monitor
		Borderless //Runs in a borderless fullscreen window (appears fullscreen)
	};

	//Platform-agnostic window singleton
	class Window {
	  public:
		//Open the window
		void Open(std::string title, glm::ivec2 initialSize, bool startVisible, WindowMode mode);
		//Close the window
		void Close();
		//Is the window open?
		bool IsOpen() {
			return isOpen;
		}
		//Check for updated state and events from the OS
		void Update();
		//Present the currently drawn image to the window
		void Present();

		//Get window size
		glm::ivec2 GetSize() {
			if(!isOpen) return glm::ivec2 {0};
			return size;
		}
		//Resizes the window (in fullscreen or borderless mode, changes resolution)
		void SetSize(glm::ivec2 newSize) {
			if(!isOpen) return;
			size = newSize;
			UpdateWindowSize();
		}

		//Enable/disable VSync
		void SetVSyncEnabled(bool value) {
			if(!isOpen) return;
			useVSync = value;
			UpdateVSyncState();
		}
		//Returns if VSync is enabled or not
		bool IsVSyncEnabled() {
			if(!isOpen) return false;
			return useVSync;
		}

		//Get native window type (returns void*, must be cast to target window type)
		void* GetNativeWindow() {
			if(!isOpen) return NULL;
			return nativeWindow;
		}

		//Set new window title
		void SetTitle(std::string title);

		//Show/hide the window
		void SetWindowVisibility(bool value) {
			if(!isOpen) return;
			isVisible = value;
			UpdateVisibilityState();
		}
		//Returns if the window is visible
		bool IsWindowVisible() {
			if(!isOpen) return false;
			return isVisible;
		}

		//Get the current window mode (returns windowed if not open)
		WindowMode GetCurrentMode() {
			if(!isOpen) return WindowMode::Window;
			return mode;
		}
		//Set the current window mode
		void SetMode(WindowMode newMode) {
			if(!isOpen) return;
			WindowMode last = mode;
			mode = newMode;
			UpdateModeState(last);
		}

		//Get window instance
		static Window* GetInstance();

	  private:
		Window()
		  : isOpen(false), mode(WindowMode::Window) {}

		//Singleton data
		static Window* instance;
		static bool instanceExists;

		bool isOpen;
		bool isVisible;

		bool useVSync;
		glm::ivec2 size;
		std::string windowTitle;

		WindowMode mode;

		//The last known window position
		//Used for switching between modes
		glm::ivec2 windowedPosition;

		void* nativeWindow;

		void UpdateVSyncState();
		void UpdateWindowSize();
		void UpdateVisibilityState();
		void UpdateModeState(WindowMode lastMode);

		//For changing the window size from implementation without generating more resize events
		friend void ChangeSize(Window* win, glm::ivec2 size) {
			win->size = size;
		}
		friend struct WindowResizer;

		NativeData* nativeData;
	};
}