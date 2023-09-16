#pragma once

#include <vector>

#include "glm/vec3.hpp"

namespace CitrusEngine {
    //Must be implemented per-rendering API
    class Mesh {
    public:
        virtual ~Mesh() {}

        //Compile mesh to a form acceptable by the rendering API
        virtual void Compile() {}

        //Release mesh assets when mesh is no longer needed
        virtual void Release() {}

        //Use this mesh
        virtual void Bind() {}

        //Don't use this mesh
        virtual void Unbind() {}

        //Get vertices
        virtual std::vector<glm::vec3> GetVertices() { return {}; }

        //Get indices
        virtual std::vector<glm::uvec3> GetIndices() { return {}; }

        static Mesh* CreateMesh(std::vector<glm::vec3> vertices, std::vector<glm::uvec3> indices);
    };
}