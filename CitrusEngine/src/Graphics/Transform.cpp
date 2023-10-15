#include "Graphics/Transform.hpp"

#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace CitrusEngine {

    void Transform::RecalculateTransformationMatrix(){
        //Calculate position move matrix
        posMat = glm::translate(glm::mat4(1.0f), pos);

        //Calculate rotation matrix
        rotMat = glm::mat4(1.0f);
        rotMat = glm::rotate(rotMat, glm::radians(rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotMat = glm::rotate(rotMat, glm::radians(rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotMat = glm::rotate(rotMat, glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

        //Calculate scale matrix
        sclMat = glm::scale(scale);

        //Combine matrices to get final matrix
        transMat = (posMat * rotMat * sclMat);
    }
}