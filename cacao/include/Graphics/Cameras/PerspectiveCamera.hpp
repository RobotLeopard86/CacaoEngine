#pragma once

#include "Camera.hpp"

#include "glm/glm.hpp"

namespace Cacao {
	//Implement a perspective camera (method explanations found in Camera.hpp)
	class PerspectiveCamera : public Camera {
	  public:
		//May only be created when the window is open, as the camera requires the window dimensions
		PerspectiveCamera(float fov = 60);

		glm::vec3 GetPosition() override {
			return position;
		}
		void SetPosition(glm::vec3 pos) override {
			position = pos;
			RecalculateViewMatrix();
		}
		glm::vec3 GetRotation() override {
			return rotation;
		}
		void SetRotation(glm::vec3 rot) override {
			rotation = rot;
			RecalculateViewMatrix();
		}

		float GetFOV() {
			return fov;
		}
		void SetFOV(float newFov) {
			fov = newFov;
			RecalculateProjectionMatrix();
		}

		glm::mat4 GetProjectionMatrix() override {
			return projectionMatrix;
		}
		glm::mat4 GetViewMatrix() override {
			return viewMatrix;
		}

		glm::vec3 GetFrontVector() {
			return frontVec;
		}
		glm::vec3 GetUpVector() {
			return upVec;
		}
		glm::vec3 GetRightVector() {
			return rightVec;
		}
		glm::vec3 GetLookTarget() {
			return position + frontVec;
		}

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