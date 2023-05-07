#include "PerspectiveCamera.h"

#include "glm/gtc/matrix_transform.hpp"

namespace CitrusEngine {
	/*
	The constructor creates the perspective projection matrix using the provided FOV (field of view)
	and aspect ratio, and a near clipping plane (where objects stop rendering when too close to the camera)
	that is extremely close so that the camera can get super close to things before they disappear.
	*/
	PerspectiveCamera::PerspectiveCamera(float fov, glm::u16vec2 aspectRatio) 
		: projectionMatrix(glm::infinitePerspective(fov, (float(aspectRatio.x) / float(aspectRatio.y)), 0.001f)), viewMatrix(1.0f),
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
		/*
		We basically draw an imaginary line out from the camera in its looking direction,
		and the look target is where the line intersects an imaginary sphere 1 unit away from the camera
		*/
		lookTarget.x = sqrt((1 - pow(sin(xRotRadians), 2)) / (1 + pow(tan(yRotRadians), 2)));
		lookTarget.y = sin(xRotRadians);
		lookTarget.z = lookTarget.x * tan(yRotRadians);

		//Add the position to our look target (originally calculated at 0, 0, 0)
		lookTarget += position;
	}
}