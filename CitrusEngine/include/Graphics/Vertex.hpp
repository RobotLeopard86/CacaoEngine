#pragma once

#include "glm/glm.hpp"

namespace CitrusEngine {
    struct Vertex {
        const glm::vec3 position;
        const glm::vec2 texCoords;
        const glm::vec3 tangent;
        const glm::vec3 bitangent;
        const glm::vec3 normal;

        Vertex(glm::vec3 position, glm::vec2 texCoords = glm::vec2(0.0f), glm::vec3 tangent = glm::vec3(0.0f), glm::vec3 bitangent = glm::vec3(0.0f), glm::vec3 normal = glm::vec3(0.0f))
            : position(position), texCoords(texCoords), tangent(tangent), bitangent(bitangent), normal(normal) {}
    };
}