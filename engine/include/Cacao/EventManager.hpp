#pragma once

#include <map>
#include <functional>
#include <vector>

#include "Event.hpp"
#include "EventConsumer.hpp"
#include "MultiFuture.hpp"
#include "DllHelper.hpp"

namespace Cacao {
	/**
	 * @brief Manages event consumers and event dispatching
	 */
	class CACAO_API EventManager {
	  public:
		/**
		 * @brief Get the instance and create one if there isn't one
		 *
		 * @return The instance
		 */
		static EventManager& Get() {
			static EventManager _instance;
			return _instance;
		}

		///@cond
		EventManager(const EventManager&) = delete;
		EventManager(EventManager&&) = delete;
		EventManager& operator=(const EventManager&) = delete;
		EventManager& operator=(EventManager&&) = delete;
		///@endcond

		/**
		 * @brief Subscribe a consumer to the given event type
		 * @details Registers this consumer to be called when an event of the given type is dispatched
		 *
		 * @param type The type of event to subscribe to
		 * @param consumer The consumer to subscribe
		 */
		void SubscribeConsumer(std::string type, const EventConsumer& consumer);

		/**
		 * @brief Unsubscribe a consumer from the given event type
		 *
		 * @param type The type of event to unsubscribe from
		 * @param consumer The consumer to unsubscribe
		 */
		void UnsubscribeConsumer(std::string type, const EventConsumer& consumer);

		/**
		 * @brief Unsubscribe all consumers of any event type
		 */
		void UnsubscribeAllConsumers();

		/**
		 * @brief Dispatch an event to all subscribed consumers of its type
		 *
		 * @warning Currently, this blocks until all consumers are done. This behavior will be changed in the future and will be replaced with a signalling API.
		 *
		 * @param event The event to dispatch
		 */
		void Dispatch(Event& event);

	  private:
		//Map of event types to list of consumers
		std::map<std::string, std::vector<EventConsumer>> consumers;

		EventManager() {}
		~EventManager() {}
	};
}
