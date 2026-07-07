#pragma once

#include "DllHelper.hpp"
#include "Asset.hpp"

#include <memory>
#include <vector>

#include "glm/glm.hpp"

namespace Cacao {
	/**
	 * @brief All data about a given vertex in a mesh
	 */
	struct CACAO_API Vertex {
		glm::vec3 position; ///<The position in local space
		glm::vec2 texCoords;///<The texture coordinates
		glm::vec3 normal;	///<The surface normal vector
		glm::vec4 tangent;	///<The surface tangent vector (stored as XYZ for the vector and W for the tangent sign)

		/**
		 * @brief Create a new vertex
		 *
		 * @param position The position in local space
		 * @param texCoords The texture coordinates (optional, defaults to {0, 0})
		 * @param normal The surface normal vector (optional, defaults to {0, 0, 0})
		 * @param tangent The surface tangent vector (optional, defaults to {0, 0, 0, 1}), stored as XYZ for vector and W for sign
		 */
		Vertex(const glm::vec3& position, const glm::vec2& texCoords = glm::vec2(0.0f), const glm::vec3& normal = glm::vec3(0.0f), const glm::vec4& tangent = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))
		  : position(position), texCoords(texCoords), normal(normal), tangent(tangent) {}
	};

	/**
	 * @brief Asset type for 3D mesh data
	 */
	class CACAO_API Mesh final : public Asset {
	  public:
		/**
		 * @brief Create a new mesh from vertex and index data
		 *
		 * @param vtx The vertices of the mesh
		 * @param idx The indices of the mesh, grouped in sets of triangles, corresponding to the vertex index in vtx
		 * @param addr The resource address to associate with the mesh
		 *
		 * @throws BadValueException If the vertex or index data is empty
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<Mesh> Create(std::vector<Vertex>&& vtx, std::vector<glm::uvec3>&& idx, const std::string& addr);

		///@cond
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&);
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&);
		///@endcond

		/**
		 * @brief Convert the mesh data into a form suitable for rendering
		 *
		 * @throws BadBakeStateException If the mesh is already baked
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Bake() override;

		/**
		 * @brief Destroy the baked representation of the asset
		 *
		 * @throws BadBakeStateException If the mesh is not baked
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Discard() override;

		///@cond
		class Impl;
		///@endcond

		~Mesh();

	  private:
		Mesh(std::vector<Vertex>&& vtx, std::vector<glm::uvec3>&& idx, const std::string& addr);
		friend class ResourceManager;
		friend class PAL;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;
	};
}