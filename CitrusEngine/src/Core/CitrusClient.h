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

        //Gets client package ID
        std::string GetPackageID() { return id; }
    protected:
        std::string id;
    private:
        static CitrusClient* instance;

        bool run;
        
        EventHandler handler;


        //Runs when client should close
        void Shutdown(Event& wce);
        //Handles non-window close events
        void HandleEvent(Event& event);
    };

    //Implemented by client
    CitrusClient* CreateClient();
}