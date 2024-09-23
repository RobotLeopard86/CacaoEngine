#pragma once

#include <map>
#include <functional>
#include <vector>

#include "Event.hpp"
#include "EventConsumer.hpp"
#include "Utilities/MultiFuture.hpp"

namespace Cacao {
	/**
	 * @brief Manages event consumers and event dispatching
	 */
	class EventManager {
	  public:
		/**
		 * @brief Subscribe a consumer to the given event type
		 * @details Registers this consumer to be called when an event of the given type is dispatched
		 *
		 * @param type The type of event to subscribe to
		 * @param consumer The consumer to subscribe
		 */
		void SubscribeConsumer(std::string type, EventConsumer* consumer);

		/**
		 * @brief Unsubscribe a consumer from the given event type
		 *
		 * @param type The type of event to unsubscribe from
		 * @param consumer The consumer to unsubscribe
		 */
		void UnsubscribeConsumer(std::string type, EventConsumer* consumer);

		/**
		 * @brief Dispatch an event to all subscribed consumers of its type
		 *
		 * @warning Currently, this blocks until all consumers are done. This behavior will be changed in the future.
		 *
		 * @param event The event to dispatch
		 */
		void Dispatch(Event& event);

		/**
		 * @brief Dispatch an event to all subscribed consumers of its type with a signal
		 *
		 * @warning Currently, this blocks until all consumers are done. This behavior will be changed in the future.
		 *
		 * @param event The event to dispatch
		 */
		std::shared_ptr<MultiFuture<void>> DispatchSignaled(Event& event);

		/**
		 * @brief Shut down the event manager and unsubscribe all consumers
		 */
		void Shutdown();

		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static EventManager* GetInstance();

	  private:
		//Map of event types to list of consumers
		std::map<std::string, std::vector<EventConsumer*>> consumers;

		EventManager();

		static EventManager* instance;
		static bool instanceExists;
	};
}
