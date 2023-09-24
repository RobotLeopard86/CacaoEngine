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
    private:
        std::map<std::string, Mesh*> meshes;

        enum Orientation {
            PosX, NegX, PosY, NegY, PosZ, NegZ
        };
    };
}