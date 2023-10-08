#pragma once

#include "Vertex.h"
#include "Shader.h"
#include "Transform.h"

#include <vector>

namespace CitrusEngine {
    //Must be implemented per-rendering API
    class Mesh {
    public:
        virtual ~Mesh() {}
        
        //Compile the mesh into a usable form for drawing
        virtual void Compile() {};
        //Release compiled assets from memory
        virtual void Release() {};
        //Draw the mesh
        virtual void Draw(Shader* shader, Transform* transform) {};
        //Draw the mesh without any modifications (for skybox rendering)
        virtual void PureDraw() {};

        //Is mesh compiled?
        bool IsCompiled() { return compiled; }

        //Create a mesh for the native platform
        static Mesh* Create(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices);
    protected:
        std::vector<Vertex> vertices;
        std::vector<glm::uvec3> indices;

        bool compiled;
    };
}