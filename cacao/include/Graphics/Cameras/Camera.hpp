#pragma once

#include "glm/glm.hpp"

#include "Events/EventSystem.hpp"
#include "Utilities/MiscUtils.hpp"
#include "3D/Orientation.hpp"

namespace Cacao {
	//Base camera type
	class Camera {
	public:
		Camera()
			: clearColor(1) {
			resizeConsumer = new EventConsumer(BIND_MEMBER_FUNC(Camera::ResizeProjectionMatrix));
			EventManager::GetInstance()->SubscribeConsumer("WindowResize", resizeConsumer);
		}

		~Camera() {
			EventManager::GetInstance()->UnsubscribeConsumer("WindowResize", resizeConsumer);
			delete resizeConsumer;
		}

		//Get and set position
		virtual glm::vec3 GetPosition() = 0;
		virtual void SetPosition(glm::vec3 pos) = 0;

		//Get and set rotation
		virtual Orientation GetRotation() = 0;
		virtual void SetRotation(Orientation rot) = 0;

		//Get projection matrix (transforms vertices according to how the camera sees them)
		virtual glm::mat4 GetProjectionMatrix() = 0;
		//Get view matrix (transforms vertices according to where the camera is looking)
		virtual glm::mat4 GetViewMatrix() = 0;

		//Update camera projection matrix for new aspect ratio
		virtual void ResizeProjectionMatrix(Event& e) = 0;
	private:
		EventConsumer* resizeConsumer;

		glm::vec4 clearColor;
	};
}