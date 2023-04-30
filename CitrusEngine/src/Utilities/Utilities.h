#pragma once

#include "Events/Events.h"

//Allows conversion of an instance member function into a form that can be called like a static function
#define BIND_MEMBER_FUNC(f) std::bind(&f, this, std::placeholders::_1)

namespace CitrusEngine {
    
    //Utilties singleton
    class Utilities {
    public:
        virtual ~Utilities() {}

        //Creates the utilities instance
        static void Create();

        //Shuts down the utilities instance
        static void Shutdown();

        //Gets the amount of elapsed time since application startup in milliseconds
        static double GetElapsedTime();
    protected:
        //Implementation of GetElapsedTime
        virtual double GetElapsedTime_Impl() = 0;

        //Creates native utilities instance
        static Utilities* CreateNativeUtilities();
    private:
        static Utilities* instance;
    };
}