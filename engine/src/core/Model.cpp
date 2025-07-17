#include "Cacao/Model.hpp"
#include "Cacao/Exceptions.hpp"

#include "assimp/Importer.hpp"
#include "assimp/config.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/texture.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"

namespace Cacao {
	struct Model::Impl {
		Assimp::Importer importer;
		const aiScene* scene;

		enum class ModelOrientation {
			PosX,
			NegX,
			PosY,
			NegY,
			PosZ,
			NegZ
		} orient;

		std::map<std::string, aiMesh*> meshIndex;
		std::map<std::string, aiTexture*> textureIndex;
	};

	Model::Model(std::vector<unsigned char>&& modelBin, const std::string& addr)
	  : Resource(addr) {
		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//Setup Assimp
		impl->importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

		/*
		Load scene

		Post-processing flags applied are:
		JoinIdenticalVertices - Makes vertices used by more than one face the same (not stored that way by default)
		CalcTangentSpace - For meshes with normals (normal mapping will later be added and require this)
		Triangulate - Ensure all polygons are triangles with 3 indices
		SortByPType - Split meshes with 2+ primitive types into submeshes with 1 primitive type (and ignores lines and points due to configuration property above)
		*/
		impl->scene = impl->importer.ReadFileFromMemory(modelBin.data(), modelBin.size(), aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType);

		//Check validity of scene
		Check<NonexistentValueException>(impl->scene != nullptr, std::string("Failed to load model data, Assimp provided error message: ") + impl->importer.GetErrorString());
		Check<NonexistentValueException>(impl->scene->HasMeshes(), "Model data does not contain any meshes!");

		//Calculate model orientation for axis correction
		int upAxis = 1, upAxisSign = 1;
		if(impl->scene->mMetaData) {
			impl->scene->mMetaData->Get<int>("UpAxis", upAxis);
			impl->scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);
		}
		if(upAxis == 0) {
			impl->orient = (upAxisSign < 1 ? Impl::ModelOrientation::NegX : Impl::ModelOrientation::PosX);
		} else if(upAxis == 1) {
			impl->orient = (upAxisSign < 1 ? Impl::ModelOrientation::NegY : Impl::ModelOrientation::PosY);
		} else if(upAxis == 2) {
			impl->orient = (upAxisSign < 1 ? Impl::ModelOrientation::NegZ : Impl::ModelOrientation::PosZ);
		}

		//Populate mesh list
		std::vector<std::string> meshIDs;
		for(unsigned int i = 0; i < impl->scene->mNumMeshes; i++) {
			aiMesh* mesh = impl->scene->mMeshes[i];
			std::string meshID = mesh->mName.length == 0 ? (std::string("Mesh") + std::to_string(i)) : std::string(mesh->mName.C_Str());
			impl->meshIndex.insert_or_assign(meshID, mesh);
			meshIDs.push_back(meshID);
		}

		//Texture scanning
		if(impl->scene->HasTextures()) {
			for(unsigned int i = 0; i < impl->scene->mNumTextures; i++) {
				aiTexture* tex = impl->scene->mTextures[i];
				impl->textureIndex.insert_or_assign(tex->mFilename.length == 0 ? (std::string("Tex") + std::to_string(i)) : std::string(tex->mFilename.C_Str()), tex);
			}
		}
	}

	const std::vector<std::string> Model::ListMeshes() {
		std::vector<std::string> accum;
		for(const auto& [id, _] : impl->meshIndex) {
			accum.push_back(id);
		}
		return accum;
	}

	const std::vector<std::string> Model::ListTextures() {
		std::vector<std::string> accum;
		for(const auto& [id, _] : impl->textureIndex) {
			accum.push_back(id);
		}
		return accum;
	}
}