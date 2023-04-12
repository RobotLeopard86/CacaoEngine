#pragma once

#include <map>
#include <functional>
#include <vector>

#include "Events.h"
#include "EventConsumer.h"

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

        void Shutdown();
    private:
        //Map of event types to list of consumers
        std::map<std::string, std::vector<EventConsumer*>> consumers;
    };
}
