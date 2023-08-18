#include "EventManager.h"

#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdexcept>

namespace CitrusEngine {

    EventManager::EventManager(){}

    void EventManager::Shutdown(){
        Logging::EngineLog(LogLevel::Info, "Event manager shutting down, unsubscribing all consumers...");

        std::map<std::string, std::vector<EventConsumer*>> consumersCopy;

        //Copy all consumer data to a map to avoid modifying the original map during unsubscription
        for(auto it = consumers.begin(); it != consumers.end(); it++){
            consumersCopy.insert_or_assign(it->first, it->second);
        }

        //Unsubscribe all consumers
        for(auto it = consumersCopy.begin(); it != consumersCopy.end(); it++){
            for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++){
                UnsubscribeConsumer(it->first, *it2);
            }
        }

        //Clear data
        consumers.clear();
        consumersCopy.clear();
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
        }
    }
}
