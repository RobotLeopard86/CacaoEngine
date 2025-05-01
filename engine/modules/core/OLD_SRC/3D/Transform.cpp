#include "3D/Transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Cacao {

	void Transform::RecalculateTransformationMatrix() {
		//Reset transformation matrix
		transMat = glm::mat4(1.0f);

		//Translate
		transMat = glm::translate(transMat, pos);

		//Rotate X
		transMat = glm::rotate(transMat, glm::radians(rot.x), {1.0, 0.0, 0.0});
		//Rotate Y
		transMat = glm::rotate(transMat, glm::radians(rot.y), {0.0, 1.0, 0.0});
		//Rotate Z
		transMat = glm::rotate(transMat, glm::radians(rot.z), {0.0, 0.0, 1.0});

		//Scale
		transMat = glm::scale(transMat, scale);
	}
}
