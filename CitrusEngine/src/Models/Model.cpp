#include "Model.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/gtx/rotate_vector.hpp"

#include "Core/Assert.h"
#include "Core/Log.h"

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

            std::vector<glm::vec3> vertices;
            std::vector<glm::u32vec3> indices;

            int upAxis = 0;
            scene->mMetaData->Get<int>("UpAxis", upAxis);
            int upAxisSign = 1;
            scene->mMetaData->Get<int>("UpAxisSign", upAxisSign);

            Logging::EngineLog(LogLevel::Info, "Up Axis: " + std::string((upAxisSign > 1 ? "+" : "-")) + std::string((upAxis == 0 ? "X" : (upAxis == 2 ? "Z" : "Y"))));

            for(int j = 0; j < assimpMesh->mNumVertices; j++){
                aiVector3D vert = assimpMesh->mVertices[j];
                glm::vec3 vertex = { vert.x, vert.y, vert.z };

                float axisCorrection = 90.0f * upAxisSign;
                
                //Apply up-axis correction
                if(upAxis == 2){
                    vertex = glm::rotateX(vertex, glm::radians(axisCorrection));
                } else if(upAxis == 0){
                    vertex = glm::rotateZ(vertex, glm::radians(axisCorrection));
                }

                vertices.push_back(vertex);
            }

            for(int j = 0; j < assimpMesh->mNumFaces; j++){
                aiFace face = assimpMesh->mFaces[j];
                indices.push_back({ face.mIndices[0], face.mIndices[1], face.mIndices[2] });
            }

            meshes.insert_or_assign(assimpMesh->mName.length == 0 ? ("Mesh" + std::to_string(i)) : std::string(assimpMesh->mName.C_Str()), std::make_pair(vertices, indices));
        }
    }

    Mesh* Model::ExtractMesh(std::string name){
        Asserts::EngineAssert(meshes.contains(name), "Model does not contain a mesh named \"" + name + "\"!");

        std::vector<glm::vec3> vertices = meshes.at(name).first;
        std::vector<glm::u32vec3> indices = meshes.at(name).second;

        return Mesh::CreateMesh(vertices, indices);
    }

    std::vector<std::string> Model::ListMeshes(){
        std::vector<std::string> keys;
        for(std::map<std::string, std::pair<std::vector<glm::vec3>, std::vector<glm::u32vec3>>>::iterator it = meshes.begin(); it != meshes.end(); ++it) {
            keys.push_back(it->first);
        }
        return keys;
    }
}