#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Cacao {
	struct GlobalsData {
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::mat4 viewProjectionMatrix;
		glm::quat camWorldRot;
		glm::vec3 camWorldPos;
		float worldTime;
		float deltaTime;
		float _paddingUnusedDoNotTouch[3];
	};
}