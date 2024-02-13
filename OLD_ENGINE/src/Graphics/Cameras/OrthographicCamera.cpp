//THIS IS CURRENTLY UNSUPPORTED!!!

/*#include "Graphics/Cameras/OrthographicCamera.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace CacaoEngine {
	//The constructor creates an orthographic camera, with the left, right, top, and bottom values determining the aspect ratio
	OrthographicCamera::OrthographicCamera(glm::vec2 projection)
		: projectionMatrix(glm::ortho(-projection.x, projection.x, -projection.y, projection.y, -1.0f, 1.0f)), viewMatrix(1.0f), position(0.0f), rotation(0.0f), projectionBox(projection) {
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

	void OrthographicCamera::RecalculateProjectionMatrix() {
		projectionMatrix = glm::ortho(-projectionBox.x, projectionBox.x, -projectionBox.y, projectionBox.y, -1.0f, 1.0f);
		//Calculate the view-projection matrix
		viewProjectionMatrix = projectionMatrix * viewMatrix;
	}

	void OrthographicCamera::ResizeProjectionMatrix(Event& e){
		WindowResizeEvent wre = Event::EventTypeCast<WindowResizeEvent>(e);
		projectionBox = (wre.size / 10);
		RecalculateProjectionMatrix();
	}
}*/