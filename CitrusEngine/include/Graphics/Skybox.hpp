#pragma once

#include "Shader.hpp"
#include "Texture.hpp"

#include "Models/Model.hpp"

namespace CitrusEngine {
    //Must be implemented per-rendering API
    class Skybox {
    public:
        virtual ~Skybox() {}

        //Draw this skybox
        virtual void Draw() = 0;

        //Create a skybox for the current rendering API
        static Skybox* Create(std::string texturePath);

        //Initialize skybox resources (shader and model) (model expected to have a mesh named "SKYBOX" in all caps)
        static void InitializeResources(std::string modelPath);
    protected:
        Transform* transform;
        Texture* tex;

        static Shader* skyboxShader;
        static Model* skybox;

        static bool staticMembersInitialized;
    };
}