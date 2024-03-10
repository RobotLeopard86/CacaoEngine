#pragma once

#include "glm/mat4x4.hpp"

#include "3D/Mesh.hpp"
#include "Graphics/Material.hpp"

namespace Cacao {
	//An object to render
	struct RenderObject {
		//Transform (must be in world space!)
		glm::mat4 transformMatrix;

		//Mesh to render
		Mesh* mesh;

		//Material to render with
		Material material;
	};
}