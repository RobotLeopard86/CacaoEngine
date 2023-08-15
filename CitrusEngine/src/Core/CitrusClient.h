#pragma once

#include <string>

#include "Events/EventSystem.h"

#include "Graphics/Window.h"

namespace CitrusEngine {

    //Singleton client
    class CitrusClient {
    public:
        CitrusClient();
        virtual ~CitrusClient();
        
        //Runs the client
        void Run();

        //Runs on startup, before Run is called
        virtual void ClientOnStartup() = 0;
        //Runs on shutdown, before engine terminates
        virtual void ClientOnShutdown() = 0;
        //Runs every dynamic tick update
        virtual void ClientOnDynamicTick(double timestep) = 0;
        //Runs every fixed tick update
        virtual void ClientOnFixedTick() = 0;

        //Handlers for client tick events
        void DynamicTickHandler(Event& e);
        void FixedTickHandler(Event& e);

        //Gets client package ID
        std::string GetPackageID() { return id; }

        //Gets event manager
        static EventManager* GetEventManager() { return eventManager; }

        //Gets window
        static Window* GetWindow() { return window; }
    protected:
        std::string id;
        glm::u16vec2 windowSize;
    private:
        static CitrusClient* instance;
        static EventManager* eventManager;
        static Window* window;

        static bool instanceExists;

        bool run;

        //Consumes WindowCloseEvents
        EventConsumer* wceConsumer;
        
        //Consumers for tick events
        EventConsumer* fixedTickConsumer;
        EventConsumer* dynamicTickConsumer;

        //Runs when client should close
        void Shutdown(Event& e);
    };
}