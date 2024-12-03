#pragma once

#include "Vertex.hpp"
#include "Transform.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Utilities/Asset.hpp"

#include <vector>
#include <future>

namespace Cacao {
	/**
	 * @brief A mesh. Implementation is backend-dependent
	 */
	class Mesh final : public Asset {
	  public:
		/**
		 * @brief Create a mesh from a list of vertices and indices
		 * @note Prefer to use AssetManager::LoadMesh over direct construction
		 *
		 * @param vertices The list of vertices
		 * @param indices The list of indices (each "index" is a triangle comprised of three positions in the vertex list)
		 */
		Mesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices);

		/**
		 * @brief Delete the mesh and release compiled data if present
		 */
		~Mesh() final {
			if(compiled) Release();
		}

		/**
		 * @brief Draw the mesh
		 *
		 * @note For use by the engine only
		 *
		 * @throws Exception If not compiled or if not called on the engine thread
		 */
		void Draw();

		/**
		 * @brief Compile the raw mesh data into a format that can be drawn asynchronously
		 *
		 * @return A future that will resolve when compilation is done
		 *
		 * @throws Exception If the mesh was already compiled
		 */
		std::shared_future<void> CompileAsync() override;

		/**
		 * @brief Compile the raw mesh data into a format that can be drawn synchronously
		 *
		 * @return A future that will resolve when compilation is done
		 *
		 * @throws Exception If the mesh was already compiled
		 */
		void CompileSync() override;

		/**
		 * @brief Delete the compiled data
		 *
		 * @throws Exception If the mesh was not compiled
		 */
		void Release() override;

		///@brief Gets the type of this asset. Needed for safe downcasting from Asset
		std::string GetType() override {
			return "MESH";
		}

	  private:
		//Backend-implemented data type
		struct MeshData;

		std::vector<Vertex> vertices;
		std::vector<glm::uvec3> indices;

		std::shared_ptr<MeshData> nativeData;
	};
}