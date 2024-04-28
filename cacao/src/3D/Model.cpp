#include "3D/Model.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/gtx/rotate_vector.hpp"

#include "Core/Exception.hpp"

#include <filesystem>
#include <ranges>

namespace Cacao {

	Model::Model(std::string filePath) {
		//Confirm that provided file path exists
		CheckException(std::filesystem::exists(filePath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), ("Cannot load nonexistent model file \"" + filePath + "\"!"))

		//Create Assimp importer
		Assimp::Importer importer;

		//Set importer to strip standalone lines and points from models (usually not needed)
		importer.SetPropertyInteger("AI_CONFIG_SBP_REMOVE", aiPrimitiveType_LINE | aiPrimitiveType_POINT);

		//Retrieve Assimp scene out of file
		/*
		Flag meanings:
		JoinIdenticalVertices - Makes vertices used by more than one face the same (not stored that way by default)
		CalcTangentSpace - For meshes with normals (normal mapping will later be added and require this)
		Triangulate - Ensure all polygons are triangles with 3 indices
		SortByPType - Split meshes with 2+ primitive types into submeshes with 1 primitive type (and ignores lines and points due to configuration property above)
		*/
		const aiScene* scene = importer.ReadFile(filePath, aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType);

		//Perform checks
		CheckException(scene != NULL, Exception::GetExceptionCodeFromMeaning("NullValue"), (std::string("Model loading failed, Assimp error: ") + importer.GetErrorString()))
		CheckException(scene->HasMeshes(), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Model file does not contain any meshes!")

		//Load meshes data
		for(int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* assimpMesh = scene->mMeshes[i];

			std::vector<Vertex> vertices;
			std::vector<glm::uvec3> indices;

			int upAxis = 1, upAxisSign = 1;
			if(scene->mMetaData) {
				scene->mMetaData->Get<int>("UpAxis", upAxis);
				scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);
			}

			ModelOrientation modelOrientation;
			//Find model orientation
			if(upAxis == 0) {
				modelOrientation = (upAxisSign < 1 ? ModelOrientation::NegX : ModelOrientation::PosX);
			} else if(upAxis == 1) {
				modelOrientation = (upAxisSign < 1 ? ModelOrientation::NegY : ModelOrientation::PosY);
			} else if(upAxis == 2) {
				modelOrientation = (upAxisSign < 1 ? ModelOrientation::NegZ : ModelOrientation::PosZ);
			}

			bool containsTexCoords = assimpMesh->HasTextureCoords(0);
			bool containsTangentsAndBitangents = assimpMesh->HasTangentsAndBitangents();
			bool containsNormals = assimpMesh->HasNormals();

			for(int j = 0; j < assimpMesh->mNumVertices; j++) {
				aiVector3D vert = assimpMesh->mVertices[j];

				glm::vec3 position = {vert.x, vert.y, vert.z};
				glm::vec2 texCoords = glm::vec2(0.0f);
				glm::vec3 tangent = glm::vec3(0.0f);
				glm::vec3 bitangent = glm::vec3(0.0f);
				glm::vec3 normal = glm::vec3(0.0f);

				if(containsTexCoords) {
					aiVector3D tc = assimpMesh->mTextureCoords[0][j];
					texCoords = {tc.x, tc.y};
				}

				if(containsTangentsAndBitangents) {
					aiVector3D tan = assimpMesh->mTangents[j];
					aiVector3D bitan = assimpMesh->mBitangents[j];
					tangent = {tan.x, tan.y, tan.z};
					bitangent = {bitan.x, bitan.y, bitan.z};
				}

				if(containsNormals) {
					aiVector3D norm = assimpMesh->mNormals[j];
					normal = {norm.x, norm.y, norm.z};
				}

				//Apply axis correction
				switch(modelOrientation) {
					case ModelOrientation::PosY:
						break;
					case ModelOrientation::NegY:
						position = glm::rotateZ(position, glm::radians(180.0f));
						break;
					case ModelOrientation::PosX:
						position = glm::rotateZ(position, glm::radians(-90.0f));
						break;
					case ModelOrientation::NegX:
						position = glm::rotateZ(position, glm::radians(90.0f));
						break;
					case ModelOrientation::PosZ:
						position = glm::rotateX(position, glm::radians(-90.0f));
						break;
					case ModelOrientation::NegZ:
						position = glm::rotateX(position, glm::radians(90.0f));
						break;
				}

				Vertex vertex {position, texCoords, tangent, bitangent, normal};
				vertices.push_back(vertex);
			}

			for(int j = 0; j < assimpMesh->mNumFaces; j++) {
				aiFace face = assimpMesh->mFaces[j];
				indices.push_back({face.mIndices[0], face.mIndices[1], face.mIndices[2]});
			}

			meshes.insert_or_assign(assimpMesh->mName.length == 0 ? ("Mesh" + std::to_string(i)) : std::string(assimpMesh->mName.C_Str()), new Mesh(vertices, indices));
		}
	}

	Model::~Model() {
		meshes.clear();
	}

	std::vector<std::string> Model::ListMeshes() {
		std::vector<std::string> keys;
		for(std::map<std::string, Mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
			keys.push_back(it->first);
		}
		return keys;
	}

	Mesh* Model::ExtractMesh(std::string id) {
		CheckException(meshes.contains(id), Exception::GetExceptionCodeFromMeaning("ContainerValue"), "Cannot extract mesh that model does not contain!")

		Mesh* mesh = meshes.at(id);
		return mesh;
	}

	bool Model::HasMesh(std::string id) {
		return meshes.contains(id);
	}
}