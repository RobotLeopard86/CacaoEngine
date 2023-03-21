#include "CitrusClient.h"
#include "Assert.h"

namespace CitrusEngine {

    //Required definitions of static members
    CitrusClient* CitrusClient::instance = nullptr;

    CitrusClient::CitrusClient(){
        //Confirm we are initializing the first client
        Asserts::EngineAssert(instance != nullptr, "Client instance already exists!");

        instance = this;
        run = true;

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

    }
}