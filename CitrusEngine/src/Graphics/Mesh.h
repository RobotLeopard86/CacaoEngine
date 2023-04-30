#pragma once

#include <vector>

#include "glm/vec3.hpp"

namespace CitrusEngine {

    class Mesh {
    public:
        Mesh(std::vector<glm::vec3> vertexBuffer, std::vector<glm::i32vec3> indexBuffer) {
            vertices = vertexBuffer;
            indices = indexBuffer;
        }

        std::vector<glm::vec3> vertices;
        std::vector<glm::i32vec3> indices;
    };
}