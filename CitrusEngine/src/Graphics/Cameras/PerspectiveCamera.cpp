#include "Graphics/Cameras/PerspectiveCamera.h"

#include "glm/gtc/matrix_transform.hpp"

namespace CitrusEngine {
	/*
	The constructor creates the perspective projection matrix using the provided FOV (field of view)
	and aspect ratio, and a near clipping plane (where objects stop rendering when too close to the camera)
	that is extremely close so that the camera can get super close to things before they disappear.
	*/
	PerspectiveCamera::PerspectiveCamera(float fov, glm::ivec2 displaySize) 
		: projectionMatrix(glm::perspective(glm::radians(fov), ((float)displaySize.x / (float)displaySize.y), 0.001f, 100000.0f)), viewMatrix(1.0f),
		position(0.0f), rotation(0.0f), viewProjectionMatrix(0.0f), lookTarget(0.0f), fov(fov), displaySize(displaySize) {}

	void PerspectiveCamera::RecalculateViewMatrix() {
		//Figure out where we are looking
		RecalculateLookTarget();
		//Look at our target from our position, with a straight up vector (where "up" is)
		viewMatrix = glm::lookAt(position, lookTarget, glm::vec3(0.0f, 1.0f, 0.0f));
		//Calculate the view-projection matrix
		viewProjectionMatrix = projectionMatrix * viewMatrix;
	}

	void PerspectiveCamera::RecalculateLookTarget() {
		//Get our X and Y rotation in radians
		float xRotRadians = glm::radians(rotation.x);
		float yRotRadians = glm::radians(rotation.y);

		//Use trigonometry to figure out where in 3D space our camera is looking if it was at 0, 0, 0
		lookTarget.x = cos(xRotRadians) * sin(yRotRadians);
		lookTarget.y = sin(xRotRadians);
		lookTarget.z = cos(xRotRadians) * cos(yRotRadians);

		//Add the position to our look target (originally calculated at 0, 0, 0)
		lookTarget += position;
	}

	void PerspectiveCamera::ResizeProjectionMatrix(Event& e){
		WindowResizeEvent wre = Event::EventTypeCast<WindowResizeEvent>(e);
		displaySize = wre.size;
		RecalculateProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateProjectionMatrix(){
		projectionMatrix = glm::perspective(glm::radians(fov), ((float)displaySize.x / (float)displaySize.y), 0.001f, 100000.0f);
		//Calculate the view-projection matrix
		viewProjectionMatrix = projectionMatrix * viewMatrix;
	}
}