#include "Cacao/PerspectiveCamera.hpp"
#include "Cacao/OrthographicCamera.hpp"
#include "Cacao/Window.hpp"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <limits>

namespace Cacao {
	Camera::Camera()
	  : resizeConsumer(std::bind(&Camera::ResizeProjectionMatrix, this, std::placeholders::_1)), clearColor(1), projectionMatrix(1.0f), viewMatrix(1.0f), viewProjectionMatrix(0.0f), position(0.0f), rotation(1.0f, glm::vec3 {0.0f}), frontVec(0.0f), upVec(0.0f), rightVec(0.0f), displaySize(Window::Get().GetContentAreaSize()) {
		EventManager::Get().SubscribeConsumer("WindowResize", resizeConsumer);
	}

	PerspectiveCamera::PerspectiveCamera(float fov)
	  : Camera(), fov(fov) {
		RecalculateProjectionMatrix();
	}

	OrthographicCamera::OrthographicCamera(float zoom)
	  : Camera(), zoom(zoom) {
		RecalculateProjectionMatrix();
	}

	void Camera::RecalculateViewMatrix() {
		//Figure out where we are looking
		RecalculateCameraVectors();

		//Look at our target from our position
		viewMatrix = glm::lookAt(position, position + frontVec, upVec);
	}

	void Camera::RecalculateCameraVectors() {
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
	}

	void Camera::ResizeProjectionMatrix(Event&) {
		displaySize = Window::Get().GetContentAreaSize();
		RecalculateProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateProjectionMatrix() {
		projectionMatrix = glm::infinitePerspective(glm::radians(fov), ((float)displaySize.x / (float)displaySize.y), 0.1f);
	}

	void OrthographicCamera::RecalculateProjectionMatrix() {
		float halfWidth = zoom;
		float halfHeight = zoom * ((float)displaySize.x / (float)displaySize.y);
		projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.1f, 10000.0f);
	}
}