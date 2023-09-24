#include "Models/Model.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/gtx/rotate_vector.hpp"

#include "Core/Assert.h"

#include <filesystem>
#include <ranges>

namespace CitrusEngine {

    Model::Model(std::string filePath){
        //Confirm that provided file path exists
        if(!std::filesystem::exists(filePath) && !std::filesystem::exists(std::filesystem::current_path().string() + "/" + filePath)){
            Asserts::EngineAssert(false, "Cannot load nonexistent model file \"" + filePath + "\"!");
        }

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
        Asserts::EngineAssert(scene != NULL, std::string("Model loading failed: ") + importer.GetErrorString());
        Asserts::EngineAssert(scene->HasMeshes(), "Model file does not contain any meshes!");

        //Load meshes data
        for(int i = 0; i < scene->mNumMeshes; i++){
            aiMesh* assimpMesh = scene->mMeshes[i];

            std::vector<Vertex> vertices;
            std::vector<glm::uvec3> indices;

            int upAxis = 0;
            scene->mMetaData->Get<int>("UpAxis", upAxis);
            int upAxisSign = 1;
            scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);

            Orientation modelOrientation;
            //Find model orientation
            if(upAxis == 0){
                modelOrientation = (upAxisSign < 1 ? Orientation::NegX : Orientation::PosX);
            } else if(upAxis == 1){
                modelOrientation = (upAxisSign < 1 ? Orientation::NegY : Orientation::PosY);
            } else if(upAxis == 2) {
                modelOrientation = (upAxisSign < 1 ? Orientation::NegZ : Orientation::PosZ);
            }

            bool containsTangentsAndBitangents = assimpMesh->HasTangentsAndBitangents();

            for(int j = 0; j < assimpMesh->mNumVertices; j++){
                aiVector3D vert = assimpMesh->mVertices[j];

                bool containsTexCoords = assimpMesh->HasTextureCoords(j);
                    
                glm::vec3 position = { vert.x, vert.y, vert.z };
                glm::vec2 texCoords = glm::vec2(0.0f);
                glm::vec3 tangent = glm::vec3(0.0f);
                glm::vec3 bitangent = glm::vec3(0.0f);

                if(containsTexCoords){
                    aiVector3D* tc = assimpMesh->mTextureCoords[j];
                    texCoords = { tc->x, tc->y };
                }

                if(containsTangentsAndBitangents){
                    aiVector3D tan = assimpMesh->mTangents[j];
                    aiVector3D bitan = assimpMesh->mBitangents[j];
                    tangent = { tan.x, tan.y, tan.z };
                    bitangent = { bitan.x, bitan.y, bitan.z };
                }

                //Apply axis correction
                switch(modelOrientation){
                case Orientation::PosY:
                    break;
                case Orientation::NegY:
                    position = glm::rotateZ(position, glm::radians(180.0f));
                    break;
                case Orientation::PosX:
                    position = glm::rotateZ(position, glm::radians(-90.0f));
                    break;
                case Orientation::NegX:
                    position = glm::rotateZ(position, glm::radians(90.0f));
                    break;
                case Orientation::PosZ:
                    position = glm::rotateX(position, glm::radians(-90.0f));
                    break;
                case Orientation::NegZ:
                    position = glm::rotateX(position, glm::radians(90.0f));
                    break;
                }

                Vertex vertex{ position, texCoords, tangent, bitangent };
                vertices.push_back(vertex);
            }

            for(int j = 0; j < assimpMesh->mNumFaces; j++){
                aiFace face = assimpMesh->mFaces[j];
                indices.push_back({ face.mIndices[0], face.mIndices[1], face.mIndices[2] });
            }

            meshes.insert_or_assign(assimpMesh->mName.length == 0 ? ("Mesh" + std::to_string(i)) : std::string(assimpMesh->mName.C_Str()), Mesh::Create(vertices, indices));
        }

        for(auto it = meshes.begin(); it != meshes.end(); it++){
            it->second->Compile();
        }
    }

    Model::~Model(){
        for(auto it = meshes.begin(); it != meshes.end(); it++){
            if(it->second->IsCompiled()){
                it->second->Release();
                delete it->second;
            }
        }
        meshes.clear();
    }

    std::vector<std::string> Model::ListMeshes(){
        std::vector<std::string> keys;
        for(std::map<std::string, Mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
            keys.push_back(it->first);
        }
        return keys;
    }

    void Model::DrawMesh(std::string id, Shader* shader, Transform* transform){
        Asserts::EngineAssert(meshes.contains(id), "Cannot draw mesh not found in model!");

        Mesh* mesh = meshes.at(id);
        mesh->Draw(shader, transform);
    }
}