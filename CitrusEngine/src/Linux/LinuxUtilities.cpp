#include "LinuxUtilities.h"

#include "GLFW/glfw3.h"

namespace CitrusEngine {

    Utilities* Utilities::CreateNativeUtilities(){
        return new LinuxUtilities();
    }

    double LinuxUtilities::GetElapsedTime() {
        return (glfwGetTime() / 1000);
    }
}