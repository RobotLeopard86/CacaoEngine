#include "Utilities.h"

namespace CitrusEngine {
    //Make utilities instance null pointer by default
    Utilities* Utilities::instance = nullptr;
    //We don't have an instance by default
    bool Utilities::instanceExists = false;

    Utilities* Utilities::GetInstance() {
        //Do we have a utilities instance yet?
        if(!instanceExists){
            //Create utilities instance
            instance = CreateNativeUtilities();
            instanceExists = true;
        }

        return instance;
    }
}