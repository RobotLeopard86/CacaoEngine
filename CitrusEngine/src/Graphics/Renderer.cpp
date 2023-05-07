#include "Renderer.h"

namespace CitrusEngine {
    //Make renderer instance null pointer by default
    Renderer* Renderer::instance = nullptr;
    //We don't have an instance by default
    bool Renderer::instanceExists = false;

    Renderer* Renderer::GetInstance() {
        //Do we have a renderer instance yet?
        if(!instanceExists){
            //Create renderer instance
            instance = CreateNativeRenderer();
            instanceExists = true;
        }

        return instance;
    }
}