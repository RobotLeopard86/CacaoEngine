#pragma once

#include "glm/vec3.hpp"

//Orientation vector type (in degrees)
struct Orientation {
	//Angle up/down from the X/Z plane
	float pitch;
	//Angle around the Y axis
	float yaw;
	//Angle around the front axis which can be created from pitch and yaw
	float roll;

	Orientation(glm::vec3 rot) {
		pitch = rot.x;
		yaw = rot.y;
		roll = rot.z;
	}

	operator glm::vec3() {
		return {pitch, yaw, roll};
	}
};