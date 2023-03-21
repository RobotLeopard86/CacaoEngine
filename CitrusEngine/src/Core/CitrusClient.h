#pragma once

#include <string>

#include "Events/Event.h"

namespace CitrusEngine {

    //Singleton client
    class CitrusClient {
    public:
        CitrusClient();
        virtual ~CitrusClient();

        void Run();

        void OnEvent(Event& event);

        std::string GetID() { return id; }
    protected:
        std::string id;
    private:
        static CitrusClient* instance;

        bool run;

        void OnWindowClose();
    };

    //Implemented by client
    CitrusClient* CreateClient();
}