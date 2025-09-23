#pragma once

#include <string>
#include <map>
#include <utility>

#include "Mesh.hpp"

namespace Cacao {
	/**
	 * @brief The representation of a model file
	 */
	class Model {
	  public:
		/**
		 * @brief Load a model from a file path
		 *
		 * @param filePath The path to load from
		 *
		 * @note Prefer to use AssetManager::LoadMesh if loading a mesh from a model file
		 *
		 * @throws Exception If the file does not exist, could not be parsed, or has no meshes
		 */
		Model(std::string filePath);

		/**
		 * @brief Destroy the model (does not destroy meshes)
		 */
		~Model();

		/**
		 * @brief List the meshes in this model
		 *
		 * @return The mesh list
		 */
		std::vector<std::string> ListMeshes();

		/**
		 * @brief Extract a mesh from this model
		 * @note The extracted mesh, if modified, will have those modifications reflected in the model's stored in-memory mesh. The mesh will not be deleted with the model.
		 *
		 * @param id The name of the mesh to extract
		 *
		 * @throws Exception If the model does not contain a mesh with the given name
		 */
		Mesh* ExtractMesh(std::string id);

		/**
		 * @brief Check if this model contains a given mesh
		 *
		 * @param id The name of the mesh to search for
		 */
		bool HasMesh(std::string id);

	  private:
		std::map<std::string, Mesh*> meshes;

		enum ModelOrientation {
			PosX,
			NegX,
			PosY,
			NegY,
			PosZ,
			NegZ
		};
	};
}