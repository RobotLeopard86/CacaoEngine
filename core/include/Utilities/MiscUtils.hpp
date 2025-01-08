#pragma once

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"

#include "Core/DllHelper.hpp"

/**
 * @brief Convert a member function of this object into a form that can be called statically
 * @warning The function must take no arguments, otherwise you will receive weird template instantiation errors that don't help
 */
#define BIND_MEMBER_FUNC(func) std::bind(&func, this, std::placeholders::_1)

namespace Cacao {
	/**
	 * @brief Result from Calculate3DVectors
	 */
	struct CACAO_API Vectors {
		glm::vec3 front, right, up;
	};

	/**
	 * @brief Calculate a front vector pointing away from the origin at the given rotation and its accompanying right and up vectors
	 * @details The rotation values must be in degrees
	 */
	CACAO_API inline Vectors Calculate3DVectors(glm::vec3 rotation) {
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

	/**
	 * @brief Fake deleter for shared pointers that doesn't delete anything
	 * @details This was made to use on Entity self-pointers, so that the shared_ptr doesn't recursively call the destructor
	 *
	 * @note This was made for internal purposes, but if it helps, feel free to use it.
	 */
	template<typename T>
	struct FakeDeleter {
		void operator()(T* e) const {}
	};
}