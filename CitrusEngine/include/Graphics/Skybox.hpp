#pragma once

#include "Shader.hpp"
#include "Textures/TextureCube.hpp"

#include "Mesh.hpp"

namespace CitrusEngine {
    //Must be implemented per-rendering API
    class Skybox {
    public:
        virtual ~Skybox() {}

        //Draw this skybox
        virtual void Draw() = 0;

        //Create a skybox for the current rendering API
        static Skybox* Create(TextureCube* tex);

		//Set up any common skybox resources
		static void CommonSetup();
		//Clean up any common skybox resources
		static void CommonCleanup();
    protected:
        TextureCube texture;

        static Shader* skyboxShader;
        static Mesh* skyboxMesh;

        static bool isSetup;
    };
}