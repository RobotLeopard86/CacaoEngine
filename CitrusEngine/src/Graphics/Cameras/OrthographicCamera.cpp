#include "OrthographicCamera.h"

#include "glm/gtc/matrix_transform.hpp"

namespace CitrusEngine {
	//The constructor creates an orthographic camera, with the left, right, top, and bottom values determining the aspect ratio
	OrthographicCamera::OrthographicCamera(float left, float right, float top, float bottom)
		: projectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), viewMatrix(1.0f), position(0.0f), rotation(0.0f) {
		viewProjectionMatrix = projectionMatrix * viewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix() {
		//Create a transform matrix based on position and rotation
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		//Invert the transform to create a view matrix
		viewMatrix = glm::inverse(transform);
		//Calculate the view-projection matrix
		viewProjectionMatrix = projectionMatrix * viewMatrix;
	}
}