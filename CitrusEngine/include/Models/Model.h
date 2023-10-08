#pragma once

#include <string>
#include <map>
#include <utility>

#include "Graphics/Mesh.h"

namespace CitrusEngine {
    //Internal representation of a model
    class Model {
    public:
        Model(std::string filePath);
        ~Model();

        //Get all mesh names
        std::vector<std::string> ListMeshes();
        //Draw a mesh by name
        void DrawMesh(std::string id, Shader* shader, Transform* transform);
        //Draw a mesh by name without any rendering state adjustments
        void DrawMeshPure(std::string id);
        //Retrieve a mesh by name (will modify value stored in model if modified and will be destroyed when the model is)
        Mesh* ExtractMesh(std::string id);
    private:
        std::map<std::string, Mesh*> meshes;

        enum Orientation {
            PosX, NegX, PosY, NegY, PosZ, NegZ
        };
    };
}