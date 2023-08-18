#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

namespace CitrusEngine {

    class Transform {
    public:
        Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
            : pos(position), rot(rotation), scale(scale) {}

        glm::vec3 pos;
        glm::vec3 rot;
        glm::vec3 scale;
    };
}