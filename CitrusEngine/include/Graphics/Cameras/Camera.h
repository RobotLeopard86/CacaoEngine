#pragma once

#include "glm/glm.hpp"

namespace CitrusEngine {
	//Base camera type
	class Camera {
	public:
		//Get and set position
		virtual glm::vec3 GetPosition() = 0;
		virtual void SetPosition(glm::vec3 pos) = 0;

		//Get and set rotation
		virtual glm::vec3 GetRotation() = 0;
		virtual void SetRotation(glm::vec3 rot) = 0;

		//Get projection matrix (transforms vertices according to how the camera sees them)
		virtual glm::mat4 GetProjectionMatrix() = 0;
		//Get view matrix (transforms vertices according to where the camera is looking)
		virtual glm::mat4 GetViewMatrix() = 0;
		//Get view-projection (combination of view and projection matrix)
		virtual glm::mat4 GetViewProjectionMatrix() = 0;
	};
}