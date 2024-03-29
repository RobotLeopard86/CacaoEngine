#pragma once

#include "Vertex.hpp"
#include "Transform.hpp"
#include "Utilities/MiscUtils.hpp"

#include <vector>
#include <future>

namespace Cacao {
    //Must be implemented per-rendering API
    class Mesh {
    public:
        Mesh(std::vector<Vertex> vertices, std::vector<glm::uvec3> indices);
		~Mesh() {
			if(compiled) Release();
			delete nativeData;
		}
        
		//Draw this mesh
		void Draw();
        //Compile the mesh into a usable form for drawing
        std::shared_future<void> Compile();
        //Release compiled assets from memory
        void Release();

        //Is mesh compiled?
        bool IsCompiled() { return compiled; }
    private:
        std::vector<Vertex> vertices;
        std::vector<glm::uvec3> indices;

        bool compiled;

		NativeData* nativeData;
    };
}