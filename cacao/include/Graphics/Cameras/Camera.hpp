#pragma once

#include "glm/glm.hpp"

#include "Events/EventSystem.hpp"
#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	/**
	 * @brief Base camera type
	 *
	 * @note Abstract, so cannot be passed by value
	 */
	class Camera {
	  public:
		///@brief Create a camera
		Camera()
		  : clearColor(1) {
			resizeConsumer = new EventConsumer(BIND_MEMBER_FUNC(Camera::ResizeProjectionMatrix));
			EventManager::GetInstance()->SubscribeConsumer("WindowResize", resizeConsumer);
		}

		///@brief Destroy a camera
		virtual ~Camera() {
			EventManager::GetInstance()->UnsubscribeConsumer("WindowResize", resizeConsumer);
			delete resizeConsumer;
		}

		/**
		 * @brief Get the position of the camera
		 *
		 * @return The camera position in world space
		 */
		virtual glm::vec3 GetPosition() = 0;

		/**
		 * @brief Set the position of the camera
		 *
		 * @param pos The new position
		 */
		virtual void SetPosition(glm::vec3 pos) = 0;

		/**
		 * @brief Get the rotation of the camera
		 *
		 * @return The camera rotation
		 */
		virtual glm::vec3 GetRotation() = 0;

		/**
		 * @brief Set the rotation of the camera
		 *
		 * @param rot The new rotation
		 */
		virtual void SetRotation(glm::vec3 rot) = 0;

		/**
		 * @brief Get the projection matrix
		 *
		 * @return The projection matrix
		 */
		virtual glm::mat4 GetProjectionMatrix() = 0;

		/**
		 * @brief Get the view matrix
		 *
		 * @return The view matrix
		 */
		virtual glm::mat4 GetViewMatrix() = 0;

		/**
		 * @brief Event handler for resizing the projection matrix when the window size changes
		 *
		 * @note For use by the engine only
		 *
		 * @param e The event object (which should be a DataEvent<glm::uvec2>)
		 */
		virtual void ResizeProjectionMatrix(Event& e) = 0;

	  private:
		EventConsumer* resizeConsumer;

		glm::vec4 clearColor;
	};
}