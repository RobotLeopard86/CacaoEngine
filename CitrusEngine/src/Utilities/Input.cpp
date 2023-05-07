#include "Input.h"

namespace CitrusEngine {
    //Make input instance null pointer by default
    Input* Input::instance = nullptr;
    //We don't have an instance by default
    bool Input::instanceExists = false;

    Input* Input::GetInstance() {
        //Do we have a input instance yet?
        if(!instanceExists){
            //Create input instance
            instance = CreateNativeInput();
            instanceExists = true;
        }

        return instance;
    }
}