#pragma once

#include "Shader.hpp"
#include "Textures/Texture2D.hpp"
#include "Mesh.hpp"

#include "glm/vec2.hpp"

#include <vector>

namespace CitrusEngine {
    //Must be implemented per-rendering API
    class Skybox {
    public:
        virtual ~Skybox() {}

        //Draw this skybox
        virtual void Draw() = 0;

        //Create a skybox for the current rendering API
        static Skybox* Create(Texture2D* tex);
		
		//Create a skybox for the current rendering API (using custom texture coordinates)
		//Texture coordinates are by vertex. Index 0 of the vector is vertex 0, index 1 is vertex 1, etc.
        static Skybox* CreateWithCustomTexCoords(Texture2D* tex, std::vector<glm::vec2> texCoords);

		//Set up any common skybox resources
		static void CommonSetup();
		//Clean up any common skybox resources
		static void CommonCleanup();
    protected:
        Texture2D* texture;
		std::vector<glm::vec2> texCoords;
		Mesh* mesh;

        static Shader* skyboxShader;

        static bool isSetup;
    };
}