#pragma once

#include <string>
#include <utility>
#include <map>

#include "glm/vec2.hpp"

namespace CitrusEngine {
    
    //Platform-agnostic window class
    class Window {
    public:
        virtual ~Window() {};

        //Creates a window
        static Window* Create(std::string title, int initialSizeX, int initialSizeY);

        //Destroys the window and also destroys the window object
        void Destroy();
        //Called every frame to update the window
        void Update();
        //Get window size (returns integer two-component vector)
        glm::i32vec2 GetSize() { return size; }
        //Resizes the window
        void SetSize(glm::i32vec2 newSize) { size = newSize; UpdateWindowSize(); }
        //Enable/disable VSync
        void SetVSyncEnabled(bool value) { useVSync = value; UpdateVSyncState(); }
        //Returns if VSync is enabled or not
        bool IsVSyncEnabled() { return useVSync; }
        //Get native window type (returns void*, must be cast to target window type)
        void* GetNativeWindow() { return nativeWindow; }
    private:
        Window(void* nativeWindow, glm::i32vec2 size, std::string title)
            : nativeWindow(nativeWindow), size(size), windowTitle(title) {
            SetVSyncEnabled(true);
        }

        bool useVSync;
        glm::i32vec2 size;
        std::string windowTitle;

        void* nativeWindow;

        static std::map<void*, Window*> nativeWindowLUT;

        void UpdateVSyncState();
        void UpdateWindowSize();
    };
}