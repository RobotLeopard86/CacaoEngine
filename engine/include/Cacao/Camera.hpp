#pragma once

#include "EventManager.hpp"
#include "DllHelper.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Cacao {
	/**
	 * @brief Base camera type
	 *
	 * @note Abstract, so cannot be passed by value
	 */
	class CACAO_API Camera {
	  public:
		///@brief Create a camera
		Camera();

		///@brief Destroy a camera
		virtual ~Camera() {
			EventManager::Get().UnsubscribeConsumer("WindowResize", resizeConsumer);
		}

		/**
		 * @brief Get the position of the camera
		 *
		 * @return The camera position in world space
		 */
		glm::vec3 GetPosition() const {
			return position;
		}

		/**
		 * @brief Set the position of the camera
		 *
		 * @param pos The new position
		 */
		void SetPosition(glm::vec3 pos) {
			position = pos;
			RecalculateViewMatrix();
		}

		/**
		 * @brief Get the rotation of the camera
		 *
		 * @return The rotation
		 */
		glm::quat GetRotation() const {
			return rotation;
		}

		/**
		 * @brief Get the rotation of the camera as Euler angles in degrees
		 *
		 * @return The rotation as Euler angles
		 */
		glm::vec3 GetRotationEuler() const {
			return glm::degrees(glm::eulerAngles(rotation));
		}

		/**
		 * @brief Set the rotation of the camera
		 *
		 * @param rot The new rotation
		 */
		void SetRotation(glm::quat rot) {
			rotation = glm::normalize(rot);
			RecalculateViewMatrix();
		}

		/**
		 * @brief Set the rotation of the camera using Euler angles in degrees
		 *
		 * @param rot The new rotation in Euler angles
		 */
		void SetRotationEuler(glm::vec3 rot) {
			rotation = glm::normalize(glm::quat(glm::radians(rot)));
			RecalculateViewMatrix();
		}

		/**
		 * @brief Get the projection matrix
		 *
		 * @return The projection matrix
		 */
		glm::mat4 GetProjectionMatrix() const {
			return projectionMatrix;
		}

		/**
		 * @brief Get the view matrix
		 *
		 * @return The view matrix
		 */
		glm::mat4 GetViewMatrix() const {
			return viewMatrix;
		}

		/**
		 * @brief Get the camera's front vector
		 *
		 * @return The unit vector pointing out at the camera's rotation
		 */
		glm::vec3 GetFrontVector() const {
			return frontVec;
		}

		/**
		 * @brief Get the camera's up vector
		 *
		 * @return The unit vector perpendicular to both the front and world up vectors
		 */
		glm::vec3 GetRightVector() const {
			return rightVec;
		}

		/**
		 * @brief Get the camera's up vector
		 *
		 * @return The unit vector perpendicular to both the front and right vectors
		 */
		glm::vec3 GetUpVector() const {
			return upVec;
		}

		/**
		 * @brief Get the point in world space where the camera is looking
		 *
		 * @return The look target (which is the position + the front vector)
		 */
		glm::vec3 GetLookTarget() const {
			return position + frontVec;
		}

		/**
		 * @brief Get the clear color used by the camera
		 *
		 * @return The camera clear color (in hexadecimal 00 to FF per component)
		 */
		glm::u8vec3 GetClearColor() const {
			return clearColor;
		}

		/**
		 * @brief Set the camera's clear color
		 *
		 * @param color The new clear color (in hexadecimal 00 to FF per component)
		 */
		void SetClearColor(glm::u8vec3 color) {
			clearColor = color;
		}

		/**
		 * @brief Event handler for resizing the projection matrix when the window size changes
		 *
		 * @note For use only by the engine internally
		 *
		 * @param e The event object (which will be casted to a DataEvent<glm::uvec2>)
		 */
		void ResizeProjectionMatrix(Event& e);

	  protected:
		EventConsumer resizeConsumer;
		glm::u8vec3 clearColor;

		glm::mat4 projectionMatrix, viewMatrix, viewProjectionMatrix;

		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 frontVec, upVec, rightVec;

		glm::uvec2 displaySize;

		//Recalculates the view matrix based on a new position and rotation
		void RecalculateViewMatrix();
		//Recalculates the projection matrix
		virtual void RecalculateProjectionMatrix() = 0;
		//Recalculates camera rotation vectors
		void RecalculateCameraVectors();
	};
}