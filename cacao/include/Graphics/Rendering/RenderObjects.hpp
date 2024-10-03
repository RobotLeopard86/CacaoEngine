#pragma once

#include "glm/mat4x4.hpp"

#include "3D/Mesh.hpp"
#include "3D/Skybox.hpp"
#include "Graphics/Material.hpp"

#include <vector>
#include <optional>

namespace Cacao {
	/**
	 * @brief Internal representation of an object to render
	 */
	struct RenderObject {
		glm::mat4 transformMatrix;///<The transformation matrix to apply
		AssetHandle<Mesh> mesh;	  ///<The mesh to draw
		Material material;		  ///<The material to draw the mesh with

		/**
		 * @brief Create a render object
		 *
		 * @param transform The transformation matrix
		 * @param mesh The mesh to draw
		 * @param mat The material to use
		 */
		RenderObject(glm::mat4 transform, AssetHandle<Mesh> mesh, Material mat)
		  : transformMatrix(transform), mesh(mesh), material(mat) {}
	};

	/**
	 * @brief Frame rendering parameters
	 */
	struct Frame {
		std::vector<RenderObject> objects;///<The list of objects to render
		glm::mat4 projection, view;		  ///<The projection and view matrices from the main camera
		AssetHandle<Skybox> skybox;		  ///<The skybox to draw (null handle means no skybox)
	};
}