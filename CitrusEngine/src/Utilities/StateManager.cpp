#include "Utilities/StateManager.hpp"

#include "Core/Assert.hpp"

namespace CitrusEngine {
    //Make state manager instance null pointer by default
    StateManager* StateManager::instance = nullptr;
    //We don't have an instance by default
    bool StateManager::instanceExists = false;

    StateManager* StateManager::GetInstance() {
        //Do we have a state manager instance yet?
        if(!instanceExists || instance == NULL){
            //Create state manager instance
            instance = new StateManager();
            instanceExists = true;
        }

        return instance;
    }

    StateManager::StateManager() {}

    void StateManager::SetActiveCamera(Camera * cam){
        activeCam = cam;
    }
}