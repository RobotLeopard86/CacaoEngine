#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#include "DllHelper.hpp"

namespace Cacao {
	/**
	 * @brief A transform defining position, rotation, and scale
	 */
	class CACAO_API Transform {
	  public:
		/**
		 * @brief Create a new transform
		 *
		 * @param position Position relative to the parent
		 * @param rotation Rotation about the center
		 * @param scale Scale from the center
		 */
		Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
		  : pos(position), rot(rotation), scale(scale), transMat(1.0) {
			RecalculateTransformationMatrix();
		}

		/**
		 * @brief Get the position
		 *
		 * @return The position
		 */
		glm::vec3 GetPosition() const {
			return pos;
		}

		/**
		 * @brief Get the rotation
		 *
		 * @return The rotation
		 */
		glm::vec3 GetRotation() const {
			return rot;
		}

		/**
		 * @brief Get the scale
		 *
		 * @return The scale
		 */
		glm::vec3 GetScale() const {
			return scale;
		}

		/**
		 * @brief Set the position
		 *
		 * @param newPos The new position
		 */
		void SetPosition(glm::vec3 newPos) {
			pos = newPos;
			RecalculateTransformationMatrix();
		}

		/**
		 * @brief Set the rotation
		 *
		 * @param newRot The new rotation
		 */
		void SetRotation(glm::vec3 newRot) {
			rot = newRot;
			RecalculateTransformationMatrix();
		}

		/**
		 * @brief Set the scale
		 *
		 * @param newScale The new scale
		 */
		void SetScale(glm::vec3 newScale) {
			scale = newScale;
			RecalculateTransformationMatrix();
		}

		/**
		 * @brief Get the matrix representing the applied transformations
		 *
		 * @return The transformation matrix
		 */
		glm::mat4 GetTransformationMatrix() const {
			return transMat;
		}

	  private:
		glm::vec3 pos, rot, scale;

		glm::mat4 transMat;

		void RecalculateTransformationMatrix();
	};
}