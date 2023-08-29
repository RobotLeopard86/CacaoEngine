#pragma once

#include <string>
#include <map>
#include <utility>

#include "Graphics/Mesh.h"

namespace CitrusEngine {
    //Internal representation of a model file (e.g. .FBX or .OBJ)
    class Model {
    public:
        Model(std::string filePath);

        //Get mesh from model (note: returns pointer of copied data, owned by callee)
        Mesh* ExtractMesh(std::string name);

        //Get all mesh names
        std::vector<std::string> ListMeshes();
    private:
        std::map<std::string, std::pair<std::vector<glm::vec3>, std::vector<glm::u32vec3>>> meshes;
    };
}