#pragma once

namespace CitrusEngine {

    //Class for controlling the backend
    class Backend {
    public:
        virtual ~Backend() {}

        //Initialize the backend
        static bool Initialize();
        //Initialize the renderer
        static bool InitRenderer();
        //Shutdown the backend (must be re-initialized before further use)
        static void Shutdown();
        //Shutdown the renderer (must be re-initialized before further use)
        static void ShutdownRenderer();

        //Check if backend is initialized or not
        static bool IsInitialized() { return initialized; }
        //Check if renderer is initialized or not
        static bool IsRendererInitialized() { return rendererInitialized; }
    private:
        static bool initialized;
        static bool rendererInitialized;
    };
}