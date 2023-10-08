#include "Graphics/Cameras/Camera.h"

#include "glad/gl.h"

namespace CitrusEngine {

    void Camera::Clear() {
        if(clearMode == ClearMode::Color) glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if(clearMode == ClearMode::Skybox){
            clearSkybox->Draw();
        }
    }
}