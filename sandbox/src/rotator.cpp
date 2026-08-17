#include "rotator.hpp"

#include "Cacao/World.hpp"
#include "glm/trigonometric.hpp"

namespace sandbox {
	void Rotator::OnDynTick(float timestep) {
		constexpr static float degPerSec = 10;
		Cacao::World* w = GetOwner().GetWorld();
		glm::quat rotDelta = glm::angleAxis(glm::radians(degPerSec * timestep), glm::vec3 {0, 1, 0});
		w->SetSkyboxRotation(rotDelta * w->GetSkyboxRotation());
	}
}