#include "Utilities.h"

#include "Core/Assert.h"
#include "Core/Log.h"

namespace CitrusEngine {
    //Make utilities instance null pointer by default
    Utilities* Utilities::instance = nullptr;

    void Utilities::Create() {
        //Make sure we don't have an active instance
        Asserts::EngineAssert(instance == nullptr, "Cannot create new utilities instance while one is already active!");

        //Create a utilities instance of the type defined by the platform
        instance = CreateNativeUtilities();

        Logging::EngineLog(LogLevel::Info, "Utilities instance created!");
    }

    void Utilities::Shutdown() {
        //Make sure we have an active instance
        Asserts::EngineAssert(instance != nullptr, "Cannot shut down uninitialized or destroyed utilities instance!");

        //Shutdown window
        instance->Shutdown();
        //Free utilities instance
        delete instance;
        //Assign null pointer to prevent another attempted destruction
        instance = nullptr;

        Logging::EngineLog(LogLevel::Info, "Utilities instance shut down!");
    }

    double Utilities::GetElapsedTime() {
        //Make sure we have an active instance
        Asserts::EngineAssert(instance != nullptr, "Cannot get elapsed time from uninitialized or destroyed utilities instance!");

        return instance->GetElapsedTime_Impl();
    }
}