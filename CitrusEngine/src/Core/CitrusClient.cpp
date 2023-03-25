#include "CitrusClient.h"
#include "Assert.h"
#include "Events/Event.h"

namespace CitrusEngine {

    //Required definitions of static members
    CitrusClient* CitrusClient::instance = nullptr;

    CitrusClient::CitrusClient(){
        //Confirm we are initializing the first client
        Asserts::EngineAssert(instance != nullptr, "Client instance already exists!");

        //Set singleton instance
        instance = this;

        //Allow the app to run
        run = true;

        //Register event callbacks
        handler = EventHandler();
        handler.RegisterCallback(EventType::WindowClose, BIND_FUNC(CitrusClient::Shutdown));
        handler.RegisterFallbackCallback(BIND_FUNC(CitrusClient::HandleEvent))

        //Set events to dispatch to OnEvent
        Event::SetDispatchMethod(BIND_FUNC(CitrusClient::OnEvent));

        //TODO: Create window
    }

    //Base client does not need a destructor
    CitrusClient::~CitrusClient() {}

    void CitrusClient::Run(){
        while(run){
            //TODO: Manage lifecycle
        }
    }

    //Runs when the application receives an event
    void CitrusClient::OnEvent(Event& event) {
        handler.Handle(event);
    }

    void CitrusClient::Shutdown(WindowCloseEvent& wce){
        wce.handled = true;

        run = false;
    }

    //TODO
    void CitrusClient::HandleEvent(Event& event) {}
}