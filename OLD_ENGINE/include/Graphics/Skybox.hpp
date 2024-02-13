#pragma once

#include "Shader.hpp"
#include "Textures/Cubemap.hpp"
#include "Mesh.hpp"

#include "glm/vec2.hpp"

#include <vector>

namespace CacaoEngine {
    //Must be implemented per-rendering API
    class Skybox {
    public:
        virtual ~Skybox() {}

        //Draw this skyboxn
        virtual void Draw() = 0;

        //Create a skybox for the current rendering API
        static Skybox* Create(Cubemap* tex);

		//Set up any common skybox resources
		static void CommonSetup();
		//Clean up any common skybox resources
		static void CommonCleanup();
    protected:
        Cubemap* texture;

        static Shader* skyboxShader;

        static bool isSetup;
    };
}