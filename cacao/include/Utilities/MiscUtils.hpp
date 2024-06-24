#pragma once

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"

//Allows conversion of an instance member function into a form that can be called like a static function
#define BIND_MEMBER_FUNC(func) std::bind(&func, this, std::placeholders::_1)

namespace Cacao {
	//Empty native data struct
	struct NativeData {};

	//Front, right, and up vector calculation result
	struct Vectors {
		glm::vec3 front, right, up;
	};
	//Caluclate the front, right, and up vectors from an rotation
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