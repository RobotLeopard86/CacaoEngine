#pragma once

#include "Camera.hpp"
#include "DllHelper.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

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

	  protected:
		float fov;
		void RecalculateProjectionMatrix() override;
	};
}