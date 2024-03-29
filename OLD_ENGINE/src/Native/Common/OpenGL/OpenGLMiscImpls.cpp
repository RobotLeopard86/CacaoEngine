#include "Graphics/Cameras/Camera.hpp"

#include "glad/gl.h"

namespace CacaoEngine {

    void Camera::Clear() {
        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}