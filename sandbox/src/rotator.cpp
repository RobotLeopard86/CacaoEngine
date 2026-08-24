#include "rotator.hpp"

#include "Cacao/World.hpp"

#include "glm/trigonometric.hpp"

namespace sandbox {
	void Rotator::OnDynTick(float timestep) {
		constexpr static float degPerSec = 10;
		std::shared_ptr<Cacao::Camera> cam = GetOwner().GetWorld()->cam;
		glm::quat rotDelta = glm::angleAxis(glm::radians(degPerSec * timestep), glm::vec3 {0, 1, 0});
		cam->SetRotation(rotDelta * cam->GetRotation());
	}
}