#pragma once

#include "Graphics/Shader.hpp"
#include "Graphics/Textures/Cubemap.hpp"
#include "Mesh.hpp"

#include "glm/vec2.hpp"

#include <vector>

namespace Cacao {
    //Must be implemented per-rendering API
    class Skybox {
    public:
		Skybox(Cubemap* tex);

        //Draw this skyboxn
        void Draw();

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