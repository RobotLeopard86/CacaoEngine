#pragma once

#include "glm/glm.hpp"

namespace Cacao {
	struct Vectors {
		glm::vec3 front, right, up;
	};

	inline Vectors Calculate3DVectors(glm::vec3 rotation) {
		//Get our X and Y rotation in radians
		float tilt = glm::radians(rotation.x);
		float pan = glm::radians(rotation.y);

		glm::vec3 frontVec, rightVec, upVec;

		//Calculate front vector
		frontVec.x = cos(tilt) * cos(pan);
		frontVec.y = sin(tilt);
		frontVec.z = cos(tilt) * sin(pan);
		frontVec = glm::normalize(frontVec);

		//Calculate right vector
		rightVec = glm::normalize(glm::cross(frontVec, {0, 1, 0}));

		//Calculate up vector
		upVec = glm::normalize(glm::cross(rightVec, frontVec));

		return {.front = frontVec, .right = rightVec, .up = upVec};
	}
}