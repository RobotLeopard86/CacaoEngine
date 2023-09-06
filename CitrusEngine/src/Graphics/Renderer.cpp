#include "Graphics/Renderer.h"

#include "Core/Assert.h"

namespace CitrusEngine {
    //Make renderer instance null pointer by default
    Renderer* Renderer::instance = nullptr;
    //We don't have an instance by default
    bool Renderer::instanceExists = false;

    Renderer* Renderer::GetInstance() {
        //Do we have a renderer instance yet?
        if(!instanceExists || instance == NULL){
            //Create renderer instance
            instance = new Renderer();
            instanceExists = true;
        }

        return instance;
    }
}