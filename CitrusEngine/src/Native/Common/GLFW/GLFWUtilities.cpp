#include "Native/Common/GLFW/GLFWUtilities.hpp"

#include "GLFW/glfw3.h"

namespace CitrusEngine {

    Utilities* Utilities::CreateNativeUtilities(){
        return new GLFWUtilities();
    }

    double GLFWUtilities::GetElapsedTime() {
        return (glfwGetTime() / 1000);
    }
}