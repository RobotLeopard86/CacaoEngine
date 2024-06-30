#include "Events/EventManager.hpp"

#include "Core/Log.hpp"
#include "Core/Exception.hpp"

#include <stdexcept>

namespace Cacao {
	//Make event manager instance null pointer by default
	EventManager* EventManager::instance = nullptr;
	//We don't have an instance by default
	bool EventManager::instanceExists = false;

	EventManager::EventManager() {}

	void EventManager::Shutdown() {
		Logging::EngineLog("Event manager shutting down!");

		std::map<std::string, std::vector<EventConsumer*>> consumersCopy;

		//Copy all consumer data to a map to avoid modifying the original map during unsubscription
		for(auto it = consumers.begin(); it != consumers.end(); it++) {
			consumersCopy.insert_or_assign(it->first, it->second);
		}

		//Unsubscribe all consumers
		for(auto it = consumersCopy.begin(); it != consumersCopy.end(); it++) {
			for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
				UnsubscribeConsumer(it->first, *it2);
			}
		}

		//Clear data
		consumers.clear();
		consumersCopy.clear();
	}

	void EventManager::SubscribeConsumer(std::string type, EventConsumer* consumer) {
		std::vector<EventConsumer*> insertValue;

		//Retrieve list of existing consumers if it exists
		if(consumers.contains(type)) {
			try {
				insertValue = consumers.at(type);
			} catch(std::out_of_range) {}
		}
		insertValue.push_back(consumer);

		//Apply changes to consumer map
		consumers.insert_or_assign(type, insertValue);
	}

	void EventManager::UnsubscribeConsumer(std::string type, EventConsumer* consumer) {
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

			for(auto iterator = insertValue.begin(); iterator != insertValue.end(); iterator++) {
				if((*iterator) == consumer) {
					consumerIndex = iterator;
					break;
				}
			}

			CheckException(consumerIndex != insertValue.end(), Exception::GetExceptionCodeFromMeaning("EventManager"), "Cannot unsubscribe consumer which was not subscribed!")

			insertValue.erase(consumerIndex);
		} else {
			CheckException(false, Exception::GetExceptionCodeFromMeaning("EventManager"), "Cannot unsubscribe consumer from event type with no consumers!")
		}

		consumers.insert_or_assign(type, insertValue);
	}

	void EventManager::Dispatch(Event& event) {
		//Check if event type has registered consumers
		if(consumers.contains(event.GetType())) {
			std::vector<EventConsumer*> eventTypeConsumers;

			//Locate consumers for event type in consumer map
			try {
				eventTypeConsumers = consumers.at(event.GetType());
			} catch(std::out_of_range) {
				CheckException(false, Exception::GetExceptionCodeFromMeaning("EventManager"), "No consumers exist for the specified event type!");
			}

			//Send event to each registered consumer
			for(EventConsumer* consumer : eventTypeConsumers) {
				consumer->Consume(event);
			}
		}
	}

	std::shared_ptr<MultiFuture<void>> EventManager::DispatchSignaled(Event& event) {
		//Create signal
		std::shared_ptr<MultiFuture<void>> signal = std::make_shared<MultiFuture<void>>();

		//Check if event type has registered consumers
		if(consumers.contains(event.GetType())) {
			std::vector<EventConsumer*> eventTypeConsumers;

			//Locate consumers for event type in consumer map
			try {
				eventTypeConsumers = consumers.at(event.GetType());
			} catch(std::out_of_range) {
				CheckException(false, Exception::GetExceptionCodeFromMeaning("EventManager"), "No consumers exist for the specified event type!");
			}

			std::vector<SignalEventConsumer*> sec;//No, not the Securities & Exchange Commision.

			//Check that each consumer is signal-processing
			for(EventConsumer* consumer : eventTypeConsumers) {
				try {
					sec.push_back(dynamic_cast<SignalEventConsumer*>(consumer));
				} catch(std::bad_cast) {
					CheckException(false, Exception::GetExceptionCodeFromMeaning("EventManager"), "Cannot dispatch signaled event to non-signal-processing consumer!");
				}
			}

			//Send event and signal to each registered consumer
			for(SignalEventConsumer* consumer : sec) {
				consumer->ConsumeWithSignal(event, *signal.get());
			}
		}

		return signal;
	}

	EventManager* EventManager::GetInstance() {
		//Do we have a event manager instance yet?
		if(!instanceExists || instance == NULL) {
			//Create event manager instance
			instance = new EventManager();
			instanceExists = true;
		}

		return instance;
	}
}
