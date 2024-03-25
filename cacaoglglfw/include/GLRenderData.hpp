#pragma once

#include "Utilities/MiscUtils.hpp"

#include "glad/gl.h"

#include <queue>

namespace Cacao {
	struct MeshAction {
		Mesh* mesh;

		enum class Action { Compile, Release };

		Action action;
	};
	
	struct GLRenderData : public NativeData {
		GLuint fbo;
		std::queue<MeshAction> meshActionQueue;
	};
}