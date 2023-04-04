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

        //Events dispatch to this method
        void OnEvent(Event& event);

        //Gets client package ID
        std::string GetPackageID() { return id; }

        static EventManager* eventManager;
    protected:
        std::string id;
    private:
        static CitrusClient* instance;

        bool run;

        //Consumes WindowCloseEvents
        EventConsumer* wceConsumer;

        //Runs when client should close
        void Shutdown(Event& e);
    };

    //Implemented by client
    CitrusClient* CreateClient();
}