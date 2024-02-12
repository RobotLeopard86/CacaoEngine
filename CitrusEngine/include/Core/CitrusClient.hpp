#pragma once

#include <string>

#include "Events/EventSystem.hpp"

#include "Utilities/StateManager.hpp"

#include "Graphics/Window.hpp"

namespace CacaoEngine {

    //Singleton client
    class CacaoClient {
    public:
        CacaoClient();
        virtual ~CacaoClient();
        
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

        //Gets window
        static Window* GetWindow() { return window; }
    protected:
        std::string windowTitle;
        glm::uvec2 windowSize;
    private:
        static CacaoClient* instance;

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