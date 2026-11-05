#include "Cacao/Transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Cacao {
	void Transform::RecalculateTransformationMatrix() {
		//Reset transformation matrix
		transMat = glm::mat4(1.0f);

		//Translate
		transMat = glm::translate(transMat, pos);

		//Rotate
		transMat *= glm::mat4_cast(rot);

		//Scale
		transMat = glm::scale(transMat, scale);
	}
}
