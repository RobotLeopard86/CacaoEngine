#pragma once

#include "Camera.hpp"
#include "DllHelper.hpp"

#include "glm/glm.hpp"

namespace Cacao {
	/**
	 * @brief Camera implementing a perspective view
	 */
	class CACAO_API PerspectiveCamera : public Camera {
	  public:
		/**
		 * @brief Create a perspective camera
		 *
		 * @param fov The FOV in degrees
		 *
		 * @throws BadInitStateException If the window is not open
		 */
		PerspectiveCamera(float fov = 60);

		/**
		 * @brief Get the position of the camera
		 *
		 * @return The camera position in world space
		 */
		glm::vec3 GetPosition() const override {
			return position;
		}

		/**
		 * @brief Set the position of the camera
		 *
		 * @param pos The new position
		 */
		void SetPosition(glm::vec3 pos) override {
			position = pos;
			RecalculateViewMatrix();
		}

		/**
		 * @brief Get the rotation of the camera
		 *
		 * @return The camera rotation
		 */
		glm::vec3 GetRotation() const override {
			return rotation;
		}

		/**
		 * @brief Set the rotation of the camera
		 *
		 * @param rot The new rotation
		 */
		void SetRotation(glm::vec3 rot) override {
			rotation = rot;
			RecalculateViewMatrix();
		}

		/**
		 * @brief Get the camera's FOV
		 *
		 * @return The FOV
		 */
		float GetFOV() const {
			return fov;
		}

		/**
		 * @brief Set the camera's FOV
		 *
		 * @param newFov The new FOV in degrees
		 */
		void SetFOV(float newFov) {
			fov = newFov;
			RecalculateProjectionMatrix();
		}

		/**
		 * @brief Get the projection matrix
		 *
		 * @return The projection matrix
		 */
		glm::mat4 GetProjectionMatrix() const override {
			return projectionMatrix;
		}

		/**
		 * @brief Get the view matrix
		 *
		 * @return The view matrix
		 */
		glm::mat4 GetViewMatrix() const override {
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
		 * @brief Event handler for resizing the projection matrix when the window size changes
		 *
		 * @note For use by the engine only
		 *
		 * @param e The event object (which should be a DataEvent<glm::uvec2>)
		 */
		void ResizeProjectionMatrix(Event& e) override;

	  private:
		glm::mat4 projectionMatrix, viewMatrix, viewProjectionMatrix;

		glm::vec3 position, rotation;
		glm::vec3 frontVec, upVec, rightVec;

		glm::uvec2 displaySize;

		float fov;

		//Recalculate the view matrix based on a new position and rotation
		void RecalculateViewMatrix();
		//Recalculate the projection matrix based on a new FOV and display size
		void RecalculateProjectionMatrix();
		//Recalculate camera rotation vectors
		void RecalculateCameraVectors();
	};
}