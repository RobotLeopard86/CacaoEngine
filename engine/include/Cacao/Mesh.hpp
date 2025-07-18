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
		const glm::vec3 position; ///<The position in local space
		const glm::vec2 texCoords;///<The texture coordinates
		const glm::vec3 tangent;  ///<The right vector in tangent space
		const glm::vec3 bitangent;///<The front vector in tangent space
		const glm::vec3 normal;	  ///<The up vector in tangent space

		/**
		 * @brief Create a new vertex
		 *
		 * @param position The position in local space
		 * @param texCoords The texture coordinates (optional, defaults to {0, 0})
		 * @param tangent The right vector in tangent space (optional, defaults to {0, 0, 0})
		 * @param bitangent The front vector in tangent space (optional, defaults to {0, 0, 0})
		 * @param normal The up vector in tangent space (optional, defaults to {0, 0, 0})
		 */
		Vertex(glm::vec3 position, glm::vec2 texCoords = glm::vec2(0.0f), glm::vec3 tangent = glm::vec3(0.0f), glm::vec3 bitangent = glm::vec3(0.0f), glm::vec3 normal = glm::vec3(0.0f))
		  : position(position), texCoords(texCoords), tangent(tangent), bitangent(bitangent), normal(normal) {}
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
		 * @param addr The resource address identifier to associate with the mesh
		 */
		static std::shared_ptr<Mesh> Create(std::vector<Vertex>&& vtx, std::vector<glm::uvec3>&& idx, const std::string& addr) {
			return std::shared_ptr<Mesh>(new Mesh(std::move(vtx), std::move(idx), addr));
		}

		///@cond
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&);
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&);
		///@endcond

		/**
		 * @brief Synchronously convert the mesh data into a form suitable for rendering
		 *
		 * @throws BadRealizeStateException If the mesh is already realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void Realize();

		/**
		 * @brief Asynchronously convert the mesh data into a form suitable for rendering
		 *
		 * @return A future that will resolve when realization is complete or fails
		 *
		 * @throws BadRealizeStateException If the mesh is already realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		std::shared_future<void> RealizeAsync();

		/**
		 * @brief Destroy the realized representation of the asset
		 *
		 * @throws BadRealizeStateException If the mesh is not realized
		 * @throws BadInitStateException If the graphics backend is not initialized or connected
		 */
		void DropRealized();

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