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
		RecalculateViewMatrix();
	}

	OrthographicCamera::OrthographicCamera(float zoom)
	  : Camera(), zoom(zoom) {
		RecalculateProjectionMatrix();
		RecalculateViewMatrix();
	}

	void Camera::RecalculateViewMatrix() {
		//Figure out where we are looking
		RecalculateCameraVectors();

		//Look at our target from our position
		viewMatrix = glm::lookAt(position, position + frontVec, upVec);
	}

	void Camera::RecalculateCameraVectors() {
		//Calculate basis matrix
		glm::mat3 basis = glm::mat3_cast(rotation);

		//Extract vectors
		rightVec = basis[0];
		upVec = basis[1];
		frontVec = basis[2];
	}

	void Camera::ResizeProjectionMatrix(Event&) {
		displaySize = Window::Get().GetContentAreaSize();
		RecalculateProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateProjectionMatrix() {
		projectionMatrix = glm::infinitePerspectiveLH_NO(glm::radians(fov), ((float)displaySize.x / (float)displaySize.y), 0.1f);
	}

	void OrthographicCamera::RecalculateProjectionMatrix() {
		float halfWidth = zoom;
		float halfHeight = zoom * ((float)displaySize.x / (float)displaySize.y);
		projectionMatrix = glm::orthoLH_NO(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.1f, 10000.0f);
	}
}