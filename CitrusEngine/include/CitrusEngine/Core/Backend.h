#pragma once

namespace CitrusEngine {

    //Class for controlling the backend
    class Backend {
    public:
        virtual ~Backend() {}

        //Initialize the backend
        static bool Initialize();
        //Shutdown the backend (must be re-initialized before further use)
        static void Shutdown();

        //Check if backend is initialized or not
        static bool IsInitialized() { return initialized; }
    private:
        static bool initialized;
    };
}