#pragma once

#include "glm/mat4x4.hpp"

#include "3D/Transform.hpp"
#include "3D/Mesh.hpp"
#include "Graphics/Material.hpp"

#include <vector>

namespace Citrus {
	//Command to draw a mesh
	struct RenderCmd {
	public:
		Mesh* mesh;
		Transform transform;
		Material material;
	};

	//Struct representing info needed for rendering
	struct RenderJob {
	public:
		//Projection and view matrices for active camera
		glm::mat4 camProjMat, camViewMat;

		//List of render commands
		std::vector<RenderCmd> renderCmds;
	};
}