#pragma once

#include <map>
#include <functional>
#include <vector>

#include "Events.hpp"
#include "EventConsumer.hpp"

namespace CitrusEngine {

    class EventManager {
    public:
        EventManager();

        //Subscribes a consumer for events of the given type to be dispatched to
        void SubscribeConsumer(std::string type, EventConsumer* consumer);

        //Unsubscribes a consumer from events of the given type
        void UnsubscribeConsumer(std::string type, EventConsumer* consumer);

        //Dispatches the event to all subscribed consumers for the appropriate event type
        void Dispatch(Event& event);

        //Shutdown the event manager and unsubscribe all consumers
        void Shutdown();

        //Get the current instance or create one if it doesn't exist
        static EventManager* GetInstance();
    private:
        //Map of event types to list of consumers
        std::map<std::string, std::vector<EventConsumer*>> consumers;

        static EventManager* instance;
        static bool instanceExists;
    };
}
