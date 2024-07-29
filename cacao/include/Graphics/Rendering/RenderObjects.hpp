#pragma once

#include "glm/mat4x4.hpp"

#include "3D/Mesh.hpp"
#include "3D/Skybox.hpp"
#include "Graphics/Material.hpp"

#include <vector>
#include <optional>

namespace Cacao {
	//An object to render
	struct RenderObject {
		//Transform (must be in world space!)
		glm::mat4 transformMatrix;

		//Mesh to render
		AssetHandle<Mesh> mesh;

		//Material to render with
		Material material;

		RenderObject(glm::mat4 transform, AssetHandle<Mesh> mesh, Material mat)
		  : transformMatrix(transform), mesh(mesh), material(mat) {}
	};

	//An entire frame
	struct Frame {
		std::vector<RenderObject> objects;
		glm::mat4 projection, view;
		AssetHandle<Skybox> skybox;
	};
}