#pragma once

#include <string>
#include <map>
#include <utility>

#include "Mesh.hpp"

namespace Citrus {
    //Internal representation of a model
    class Model {
    public:
        Model(std::string filePath);
        ~Model();

        //Get all mesh names
        std::vector<std::string> ListMeshes();
        //Retrieve a mesh by name (will modify value stored in model if modified and will be destroyed when the model is)
        Mesh* ExtractMesh(std::string id);
    private:
        std::map<std::string, Mesh*> meshes;

        enum ModelOrientation {
            PosX, NegX, PosY, NegY, PosZ, NegZ
        };
    };
}