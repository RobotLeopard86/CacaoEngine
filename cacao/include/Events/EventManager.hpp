#pragma once

#include <map>
#include <functional>
#include <vector>

#include "Event.hpp"
#include "EventConsumer.hpp"

#include "BS_thread_pool.hpp"

namespace Cacao {
	//Manager for the event system
	class EventManager {
	  public:
		//Subscribes a consumer for events of the given type to be dispatched to
		void SubscribeConsumer(std::string type, EventConsumer* consumer);

		//Unsubscribes a consumer from events of the given type
		void UnsubscribeConsumer(std::string type, EventConsumer* consumer);

		//Dispatches an event to all subscribed consumers for the appropriate event type
		void Dispatch(Event& event);

		//Dispatches an event to all subscribed consumers for the appropriate event type
		//Also returns a signal that can be waited on to know when all consumers have finished
		//This requires that all registered consumers are signal-processing, if not this function will throw an exception
		std::shared_ptr<EventSignal> DispatchSignaled(Event& event);

		//Shutdown the event manager and unsubscribe all consumers
		void Shutdown();

		//Get the current instance or create one if it doesn't exist
		static EventManager* GetInstance();

	  private:
		//Map of event types to list of consumers
		std::map<std::string, std::vector<EventConsumer*>> consumers;

		EventManager();

		static EventManager* instance;
		static bool instanceExists;
	};
}
