#include "selfspinner.hpp"

#include "Cacao/World.hpp"

#include "glm/trigonometric.hpp"

#include "sandbox.astra.hpp"
#include "cacaoengine.astra.hpp"

namespace sandbox {
	void SelfSpinner::OnDynTick(float timestep) {
		constexpr static float degPerSec = -60;
		Transform transform = GetOwner()->GetLocalTransform();
		glm::quat rotDelta = glm::angleAxis(glm::radians(degPerSec * timestep), glm::vec3 {0, 1, 0});
		transform.SetRotation(rotDelta * transform.GetRotation());
		GetOwner()->SetLocalTransform(transform);
	}
}