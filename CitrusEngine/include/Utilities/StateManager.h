#pragma once

#include "Graphics/Cameras/Camera.h"

namespace CitrusEngine {
    //Singleton state manager for the engine
    class StateManager {
    public:
        //Set the active camera
        void SetActiveCamera(Camera* cam);
        //Get the active camera
        Camera* GetActiveCamera() { return activeCam; }

        //Get the current instance or create one if it doesn't exist
        static StateManager* GetInstance();
    private:
        static StateManager* instance;
        static bool instanceExists;

        Camera* activeCam;

        StateManager();
    };
}