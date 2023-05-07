#pragma once

#include "Camera.h"

namespace CitrusEngine {
	//Implement an orthographic camera (method explanations found in Camera.h)
	class OrthographicCamera : public Camera {
	public:
		OrthographicCamera(float left, float right, float top, float bottom);

		glm::vec3 GetPosition() override { return position; }
		void SetPosition(glm::vec3 pos) override { position = pos; position.y *= -1; RecalculateViewMatrix(); }
		glm::vec3 GetRotation() override { return glm::vec3(0, 0, rotation); }
		void SetRotation(glm::vec3 rot) override { rotation = rot.z; RecalculateViewMatrix(); }

		glm::mat4 GetProjectionMatrix() override { return projectionMatrix; }
		glm::mat4 GetViewMatrix() override { return viewMatrix; }
		glm::mat4 GetViewProjectionMatrix() override { return viewProjectionMatrix; }
	private:
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 viewProjectionMatrix;

		glm::vec3 position;
		//Orthographic cameras only use Z rotation
		float rotation;

		void RecalculateViewMatrix();
	};
}