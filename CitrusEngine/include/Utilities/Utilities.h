#pragma once

#include "Events/Events.h"

//Allows conversion of an instance member function into a form that can be called like a static function
#define BIND_MEMBER_FUNC(f) std::bind(&f, this, std::placeholders::_1)

namespace CitrusEngine {
    //Utilties singleton
    class Utilities {
    public:
        virtual ~Utilities() {}

        //Gets the amount of elapsed time since application startup in milliseconds
        virtual double GetElapsedTime() = 0;

        //Get the current instance or create one if it doesn't exist
        static Utilities* GetInstance();
    protected:
        //Creates renderer for the native platform (implemented by subclasses)
        static Utilities* CreateNativeUtilities();

        //Protected constructor so only subclasses can call it
        Utilities() {}
    private:
        static Utilities* instance;
        static bool instanceExists;
    };
}