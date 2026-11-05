#pragma once

#include "Camera.hpp"
#include "DllHelper.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Cacao {
	/**
	 * @brief Camera implementing a orthographic view
	 */
	class CACAO_API OrthographicCamera : public Camera {
	  public:
		/**
		 * @brief Create a orthographic camera
		 *
		 * @param zoom The zoom-in factor for the camera (default: 1, <1 for zoom out, >1 for zoom in)
		 *
		 * @throws BadInitStateException If the window is not open
		 */
		OrthographicCamera(float zoom = 1);

		/**
		 * @brief Get the camera's zoom factor
		 *
		 * @return The zoom factor
		 */
		float GetZoomFactor() const {
			return zoom;
		}

		/**
		 * @brief Set the camera's zoom factor
		 *
		 * @param newZoom The new zoom factor for the camera (default: 1, <1 for zoom out, >1 for zoom in)
		 */
		void SetZoomFactor(float newZoom) {
			zoom = newZoom;
			RecalculateProjectionMatrix();
		}

	  private:
		float zoom;
		void RecalculateProjectionMatrix() override;
	};
}