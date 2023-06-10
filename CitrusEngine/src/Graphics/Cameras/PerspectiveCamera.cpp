#include "PerspectiveCamera.h"

#include "glm/gtc/matrix_transform.hpp"

namespace CitrusEngine {
	/*
	The constructor creates the perspective projection matrix using the provided FOV (field of view)
	and aspect ratio, and a near clipping plane (where objects stop rendering when too close to the camera)
	that is extremely close so that the camera can get super close to things before they disappear.
	*/
	PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio) 
		: projectionMatrix(glm::infinitePerspective(fov, aspectRatio, 0.001f)), viewMatrix(1.0f),
		position(0.0f), rotation(0.0f), viewProjectionMatrix(0.0f), lookTarget(0.0f) {}

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
		lookTarget.x = sin(yRotRadians);
		lookTarget.y = sqrt((1 / pow(cos(xRotRadians), 2)) - 1);
		lookTarget.z = cos(yRotRadians);

		//Negate y look target if X rotation less than 0 (quirk of the math)
		if(rotation.x < 0){
			lookTarget.y *= -1;
		}

		//Add the position to our look target (originally calculated at 0, 0, 0)
		lookTarget += position;
	}
}