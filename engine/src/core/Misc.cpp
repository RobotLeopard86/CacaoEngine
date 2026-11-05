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

#include "cacaoengine.astra.hpp"

namespace Cacao {
	ImplAccessor::ImplAccessor() {}

	CACAOST_GET(ImplAccessor)

	Script::~Script() = default;

	//VERY BIG HACK: this forces the Script class to actually generate a vtable
	struct ScriptVTableIncluder {
		ScriptVTableIncluder() {
			delete new Script();
		}
	};
	__attribute__((used)) ScriptVTableIncluder hack;

	void Script::OnEnable() {
		//Add to script list
		{
			std::lock_guard lk(IMPL(TickController).tickScriptsMtx);
			IMPL(TickController).tickScripts.insert(this);
		}

		//Forward to subclass behavior
		OnEnabled();
	}

	void Script::OnDisable() {
		//Remove from script list
		{
			std::lock_guard lk(IMPL(TickController).tickScriptsMtx);
			IMPL(TickController).tickScripts.erase(this);
		}

		//Forward to subclass behavior
		OnDisabled();
	}

	void MeshRenderer::OnEnable() {
		//Add to renderer list
		std::lock_guard lk(IMPL(GPUManager).renderersMtx);
		IMPL(GPUManager).renderers.insert(this);
	}

	void MeshRenderer::OnDisable() {
		//Remove from renderer list
		std::lock_guard lk(IMPL(GPUManager).renderersMtx);
		IMPL(GPUManager).renderers.erase(this);
	}

}