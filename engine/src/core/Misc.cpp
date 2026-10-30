//This file is for things that are small enough not to warrant their own file so they go here instead

#include "Cacao/Mesh.hpp"
#include "Cacao/MeshRenderer.hpp"
#include "Cacao/Script.hpp"
#include "impl/GPUManager.hpp"
#include "impl/TickController.hpp"
#include "ImplAccessor.hpp"
#include "SingletonGet.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

namespace Cacao {
	ImplAccessor::ImplAccessor() {}

	CACAOST_GET(ImplAccessor)

	Script::~Script() = default;
	astra::TypeId Cacao::Script::ASTRA__gettypeid() const {
		return astra::TypeIdFactory<Cacao::Script>::get();
	}

	void Script::OnEnable() {
		//TODO

		//Forward to subclass behavior
		OnEnabled();
	}

	void Script::OnDisable() {
		//TODO

		//Forward to subclass behavior
		OnDisabled();
	}

	void MeshRenderer::OnEnable() {
		//TODO
	}

	void MeshRenderer::OnDisable() {
		//TODO
	}

}