#include "Graphics/Cameras/PerspectiveCamera.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "Graphics/Window.hpp"
#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	/*
	The constructor creates the perspective projection matrix using the provided FOV (field of view)
	and aspect ratio, and a near clipping plane (where objects stop rendering when too close to the camera)
	that is extremely close so that the camera can get super close to things before they disappear.
	*/
	PerspectiveCamera::PerspectiveCamera(float fov)
	  : projectionMatrix(1.0f), viewMatrix(1.0f), viewProjectionMatrix(0.0f), position(0.0f), rotation(0.0f), frontVec(0.0f), upVec(0.0f), rightVec(0.0f), displaySize(Window::GetInstance()->GetSize()), fov(fov) {
		RecalculateProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateViewMatrix() {
		//Figure out where we are looking
		RecalculateCameraVectors();
		//Look at our target from our position
		viewMatrix = glm::lookAt(position, position + frontVec, upVec);
	}

	void PerspectiveCamera::RecalculateCameraVectors() {
		//Calculate 3D vectors
		Vectors vecs = Calculate3DVectors(rotation);
		frontVec = vecs.front;
		rightVec = vecs.right;
		upVec = vecs.up;
	}

	void PerspectiveCamera::ResizeProjectionMatrix(Event& e) {
		displaySize = Window::GetInstance()->GetSize();
		RecalculateProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateProjectionMatrix() {
		projectionMatrix = glm::infinitePerspective(glm::radians(fov), ((float)displaySize.x / (float)displaySize.y), 0.1f);
	}
}