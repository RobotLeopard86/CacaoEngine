#include "Graphics/Cameras/PerspectiveCamera.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace CitrusEngine {
	/*
	The constructor creates the perspective projection matrix using the provided FOV (field of view)
	and aspect ratio, and a near clipping plane (where objects stop rendering when too close to the camera)
	that is extremely close so that the camera can get super close to things before they disappear.
	*/
	PerspectiveCamera::PerspectiveCamera(float fov, glm::ivec2 displaySize) 
		: projectionMatrix(glm::perspective(glm::radians(fov), ((float)displaySize.x / (float)displaySize.y), 0.001f, 100000.0f)), viewMatrix(1.0f),
		position(0.0f), rotation(glm::vec3{0.0f}), viewProjectionMatrix(0.0f), frontVec(0.0f), upVec(0.0f), rightVec(0.0f), fov(fov), displaySize(displaySize) {}

	void PerspectiveCamera::RecalculateViewMatrix() {
		//Figure out where we are looking
		RecalculateCameraVectors();
		//Look at our target from our position, with a straight up vector (where "up" is)
		viewMatrix = glm::lookAt(position, position + frontVec, upVec);
		//Calculate the view-projection matrix
		viewProjectionMatrix = projectionMatrix * viewMatrix;
	}

	void PerspectiveCamera::RecalculateCameraVectors() {
		//Get our X and Y rotation in radians
		float tilt = glm::radians(rotation.tilt);
		float pan = glm::radians(rotation.pan);

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