#pragma once

#include <string>
#include <map>
#include <utility>

#include "Mesh.hpp"

namespace Cacao {
    //Internal representation of a model
    class Model {
    public:
        Model(std::string filePath);
        ~Model();

        //Get all mesh names
        std::vector<std::string> ListMeshes();
        //Retrieve a mesh by name (will modify value stored in model if modified but will NOT be deleted when model is)
        Mesh* ExtractMesh(std::string id);
		//Check if a mesh is in the model
		bool HasMesh(std::string id);
    private:
        std::map<std::string, Mesh*> meshes;

        enum ModelOrientation {
            PosX, NegX, PosY, NegY, PosZ, NegZ
        };
    };
}