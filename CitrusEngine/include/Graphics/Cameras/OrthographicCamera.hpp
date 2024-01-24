#pragma once

//THIS IS CURRENTLY UNSUPPORTED!!!

/*#include "Camera.hpp"

namespace CitrusEngine {
	//Implement an orthographic camera (method explanations found in Camera.hpp)
	class OrthographicCamera : public Camera {
	public:
		OrthographicCamera(glm::vec2 projection);

		glm::vec3 GetPosition() override { return position; }
		void SetPosition(glm::vec3 pos) override { position = pos; position.y *= -1; RecalculateViewMatrix(); }
		Orientation GetRotation() override { return glm::vec3(0, 0, rotation); }
		void SetRotation(Orientation rot) override { rotation = rot.roll; RecalculateViewMatrix(); }

		glm::mat4 GetProjectionMatrix() override { return projectionMatrix; }
		glm::mat4 GetViewMatrix() override { return viewMatrix; }
		glm::mat4 GetViewProjectionMatrix() override { return viewProjectionMatrix; }

		void ResizeProjectionMatrix(Event& e) override;
	private:
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 viewProjectionMatrix;

		glm::vec2 projectionBox;

		glm::vec3 position;
		//Orthographic cameras only use roll
		float rotation;

		void RecalculateViewMatrix();
		void RecalculateProjectionMatrix();
	};
}*/