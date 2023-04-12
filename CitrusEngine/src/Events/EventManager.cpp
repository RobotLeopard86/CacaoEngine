#include "EventManager.h"

#include "Core/Log.h"
#include "Core/Utilities.h"
#include "Core/Assert.h"

#include <stdexcept>

namespace CitrusEngine {

    EventManager::EventManager(){}

    void EventManager::Shutdown(){
        Logging::EngineLog(LogLevel::Info, "Event manager shutting down, unsubscribing all consumers...");

        std::vector<std::string> registeredTypes;

        //Unsubscribe all consumers
        for(const auto& [type, eventTypeConsumers] : consumers){
            for(EventConsumer* consumer : eventTypeConsumers){
                UnsubscribeConsumer(type, consumer);
            }
            registeredTypes.push_back(type);
        }

        //Delete all empty entries once consumers are unsubscribed
        for(std::string type : registeredTypes){
            consumers.erase(type);
        }
    }

    void EventManager::SubscribeConsumer(std::string type, EventConsumer* consumer){

        std::vector<EventConsumer*> insertValue;

        //Retrieve list of existing consumers if it exists
        if(consumers.contains(type)){
            try {
                insertValue = consumers.at(type);
            } catch(std::out_of_range) {}
        }
        insertValue.push_back(consumer);

        //Apply changes to consumer map
        consumers.insert_or_assign(type, insertValue);
    }

    void EventManager::UnsubscribeConsumer(std::string type, EventConsumer* consumer){
        std::vector<EventConsumer*> insertValue;

        //Retrieve list of existing consumers
        if(consumers.contains(type)) {
            try {
                insertValue = consumers.at(type);
            } catch(std::out_of_range) {
                return;
            }

            //Get index of selected consumer
            std::vector<EventConsumer*>::iterator consumerIndex = insertValue.end();

            for(auto iterator = insertValue.begin(); iterator != insertValue.end(); iterator++){
                if((*iterator) == consumer){
                    consumerIndex = iterator;
                    break;
                }
            }

            if(consumerIndex == insertValue.end()){
                Logging::EngineLog(LogLevel::Error, "Cannot unsubscribe consumer which was not subscribed!");
                return;
            }

            insertValue.erase(consumerIndex);
        } else {
            Logging::EngineLog(LogLevel::Error, "Cannot unsubscribe consumer from event type with no consumers!");
            return;
        }
        
        consumers.insert_or_assign(type, insertValue);
    }

    void EventManager::Dispatch(Event& event){
        //Check if event type has registered consumers
        if(consumers.contains(event.GetType())){
            std::vector<EventConsumer*> eventTypeConsumers;

            //Locate consumers for event type in consumer map
            try {
                eventTypeConsumers = consumers.at(event.GetType());
            } catch(std::out_of_range){
                return;
            }

            //Send event to each registered consumer
            for(EventConsumer* consumer : eventTypeConsumers){
                consumer->Consume(event);
            }
        } else {
            Logging::EngineLog(LogLevel::Warn, "No consumers registered for dispatched event, not dispatching.");
        }
    }
}
