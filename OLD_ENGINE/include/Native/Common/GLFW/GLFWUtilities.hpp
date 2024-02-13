#pragma once

#include "Utilities/Utilities.hpp"

namespace CacaoEngine {
    
    //GLFW implementation of Utilities (see Utilities.hpp for method details)
    class GLFWUtilities : public Utilities {
    public:
        GLFWUtilities() {}

        double GetElapsedTime() override;
    };
}