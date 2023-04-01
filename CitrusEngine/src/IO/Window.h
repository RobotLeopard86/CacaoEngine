#pragma once

#include <string>

namespace CitrusEngine {
    
    //Window singleton
    class Window {
    public:
        //Creates a window
        static void Create(std::string title, int initialSizeX, int initialSizeY);

        static void Destroy();
    private:
        static Window* instance;
    };
}