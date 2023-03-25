#pragma once

#include <string>

#include "Events/Event.h"

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

        //Gets app ID
        std::string GetID() { return id; }
    protected:
        std::string id;
    private:
        static CitrusClient* instance;

        bool run;
        EventHandler handler;

        //Runs when client should close
        void Shutdown();
        //Handles non-window close events
        void HandleEvent(Event& event);
    };

    //Implemented by client
    CitrusClient* CreateClient();
}