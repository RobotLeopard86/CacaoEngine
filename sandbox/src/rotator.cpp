#include "rotator.hpp"

#include "Cacao/World.hpp"

namespace sandbox {
	void Rotator::OnDynTick(std::chrono::seconds timestep) {
		constexpr static float degPerSec = 5;
		Cacao::World* w = GetOwner().GetWorld();
		glm::vec3 rotation = w->GetSkyboxRotationEuler();
		rotation.y += (degPerSec * timestep.count());
		w->SetSkyboxRotationEuler(rotation);
	}
}