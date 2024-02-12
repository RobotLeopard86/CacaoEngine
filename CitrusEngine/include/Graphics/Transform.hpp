#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

namespace CacaoEngine {

    class Transform {
    public:
        Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
            : pos(position), rot(rotation), scale(scale), transMat(1.0) {
				RecalculateTransformationMatrix();
			}

        //Self-explanatory getters and setters
        glm::vec3 GetPosition() { return pos; }
        glm::vec3 GetRotation() { return rot; }
        glm::vec3 GetScale() { return scale; }

        void SetPosition(glm::vec3 newPos) { pos = newPos; RecalculateTransformationMatrix(); }
        void SetRotation(glm::vec3 newRot) { rot = newRot; RecalculateTransformationMatrix(); }
        void SetScale(glm::vec3 newScale) { scale = newScale; RecalculateTransformationMatrix(); }

        glm::mat4 GetTransformationMatrix() { return transMat; }
    private:
        glm::vec3 pos, rot, scale;

        glm::mat4 transMat;

        void RecalculateTransformationMatrix();
    };
}