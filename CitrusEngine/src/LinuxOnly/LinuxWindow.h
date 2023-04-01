#pragma once

#include "IO/Window.h"

namespace CitrusEngine {

    //GNU/Linux implementation of Window
    class LinuxWindow : public Window {
    public:
        LinuxWindow(std::string title, int intialSizeX, int initialSizeY) {}
    };
}