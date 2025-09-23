#pragma once

#include "DllHelper.hpp"
#include "Resource.hpp"
#include "Tex2D.hpp"
#include "Mesh.hpp"

namespace Cacao {
	/**
	 * @brief A resource representation of a model file, containing meshes and textures found within
	 */
	class CACAO_API Model final : public Resource {
	  public:
		/**
		 * @brief Create a new model from data
		 *
		 * @param modelBin A blob of model data encoded as FBX Binary, glTF Binary (.glb), Collada, or Wavefront OBJ
		 * @param addr The resource address to associate with the model
		 *
		 * @throws BadValueException If the model data is empty
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<Model> Create(std::vector<unsigned char>&& modelBin, const std::string& addr) {
			return std::shared_ptr<Model>(new Model(std::move(modelBin), addr));
		}

		/**
		 * @brief Get a list of stored meshes
		 *
		 * @return Mesh ID list
		 */
		const std::vector<std::string> ListMeshes();

		/**
		 * @brief Get a list of stored textures
		 *
		 * @return Texture ID list
		 */
		const std::vector<std::string> ListTextures();

		/**
		 * @brief Retrieve a stored Mesh by its ID
		 *
		 * @param id The ID of the mesh to retrieve
		 *
		 * @return A handle to the mesh resource
		 *
		 * @throws NonexistentValueException If the provided ID is not found in meshList
		 */
		std::shared_ptr<Mesh> GetMesh(const std::string& id);

		/**
		 * @brief Retrieve a stored Tex2D by its ID
		 *
		 * @param id The ID of the texture to retrieve
		 *
		 * @return A handle to the texture resource
		 *
		 * @throws NonexistentValueException If the provided ID is not found in texList
		 */
		std::shared_ptr<Tex2D> GetTexture(const std::string& id);

		///@cond
		struct Impl;
		///@endcond

		~Model();

	  private:
		Model(std::vector<unsigned char>&& modelBin, const std::string& addr);
		friend class ResourceManager;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;
	};
}