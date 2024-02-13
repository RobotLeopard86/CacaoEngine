#pragma once

#include <string>
#include <utility>
#include <map>

#include "glm/vec2.hpp"

namespace Cacao {
    //Platform-agnostic window singleton
    class Window {
    public:
        //Open the window
		void Open(std::string title, int initialSizeX, int initialSizeY);
        //Close the window
        void Close();
		//Is the window open?
		bool IsOpen() { return isOpen; }
        //Called every frame to update the window
        void Update();
        //Get window size (returns integer two-component vector)
        glm::ivec2 GetSize() { if(!isOpen) return glm::ivec2{0}; return size; }
        //Resizes the window
        void SetSize(glm::ivec2 newSize) { if(!isOpen) return; size = newSize; UpdateWindowSize(); }
        //Enable/disable VSync
        void SetVSyncEnabled(bool value) { if(!isOpen) return; useVSync = value; UpdateVSyncState(); }
        //Returns if VSync is enabled or not
        bool IsVSyncEnabled() { if(!isOpen) return false; return useVSync; }
        //Get native window type (returns void*, must be cast to target window type)
        void* GetNativeWindow() { if(!isOpen) return NULL; return nativeWindow; }

		//Get window instance
		static Window* GetInstance();
    private:
        Window() : isOpen(false) {}

		//Singleton data
		static Window* instance;
		static bool instanceExists;

		bool isOpen;

        bool useVSync;
        glm::ivec2 size;
        std::string windowTitle;

        void* nativeWindow;

        void UpdateVSyncState();
        void UpdateWindowSize();
    };
}