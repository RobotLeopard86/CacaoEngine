#pragma once

#include <string>
#include <utility>

#include "glm/vec2.hpp"

namespace CitrusEngine {
    
    //Window singleton
    class Window {
    public:
        virtual ~Window() {};

        //Creates a window
        static void Create(std::string title, int initialSizeX, int initialSizeY);

        //Destroys the window
        static void Destroy();

        //Called every frame to update the window
        static void Update();
        //Get pointer to underlying window type
        static void* GetNativeWindow();
        //Get window size (returns integer two-component vector)
        static glm::i32vec2 GetSize();
        //Resizes the window
        static void SetSize(glm::i32vec2 newSize);
        //Enable/disable VSync
        static void SetVSyncEnabled(bool value);
        //Returns if VSync is enabled or not
        static bool IsVSyncEnabled();
    protected:
        //Implementation of Update
        virtual void Update_Impl() = 0;
        //Implementation of GetNativeWindow
        virtual void* GetNativeWindow_Impl() = 0;
        //Implementation of GetSize
        virtual glm::i32vec2 GetSize_Impl() = 0;
        //Implemntation of SetSize
        virtual void SetSize_Impl(glm::i32vec2 newSize) {}
        //Implementation of SetVSyncEnabled
        virtual void SetVSyncEnabled_Impl(bool value) {}
        //Implementation of IsVSyncEnabled
        virtual bool IsVSyncEnabled_Impl() = 0;

        //Runs when window should shut down
        virtual void Shutdown() = 0;

        //Creates window for the native platform
        static Window* CreateNativeWindow(std::string title, int initialSizeX, int initialSizeY);
    private:
        static Window* instance;
    };
}