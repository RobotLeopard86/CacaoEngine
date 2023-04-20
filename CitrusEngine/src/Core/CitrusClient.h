#pragma once

#include <string>

#include "Events/EventSystem.h"

namespace CitrusEngine {

    //Singleton client
    class CitrusClient {
    public:
        CitrusClient();
        virtual ~CitrusClient();
        
        //Runs the client
        void Run();

        //Gets client package ID
        std::string GetPackageID() { return id; }

        //Gets event manager
        static EventManager* GetEventManager() { return eventManager; }
    protected:
        std::string id;
    private:
        static CitrusClient* instance;
        static EventManager* eventManager;

        bool run;

        //Consumes WindowCloseEvents
        EventConsumer* wceConsumer;

        //Runs when client should close
        void Shutdown(Event& e);
    };
}