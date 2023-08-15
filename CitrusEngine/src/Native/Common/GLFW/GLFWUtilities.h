#pragma once

#include "Utilities/Utilities.h"

namespace CitrusEngine {
    
    //GLFW implementation of Utilities (see Utilities.h for method details)
    class GLFWUtilities : public Utilities {
    public:
        GLFWUtilities() {}

        double GetElapsedTime() override;
    };
}