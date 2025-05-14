#pragma once

#include "Event.hpp"
#include "Exceptions.hpp"
#include "MultiFuture.hpp"
#include "DllHelper.hpp"

#include <functional>
#include <vector>

#include "crossguid/guid.hpp"

namespace Cacao {
	/**
	 * @brief Simple wrapper for a function that consumes an event
	 */
	class CACAO_API EventConsumer {
	  public:
		/**
		 * @brief Create an event consumer
		 *
		 * @param consumer The function to call when an event is received
		 */
		EventConsumer(std::function<void(Event&)> consumer)
		  : consumer(consumer), guid(xg::newGuid()) {}

		/**
		 * @brief Create an empty event consumer that does nothing
		 */
		EventConsumer()
		  : consumer([](Event&) {}), guid(xg::newGuid()) {}

		/**
		 * @brief Consume an event
		 * @details Forwards the event to the consuming function
		 */
		void Consume(Event& event) {
			consumer(event);
		}

		/**
		 * @brief Check if two EventConsumers are equal
		 * @details Compares GUIDs (which should basically never overlap)
		 */
		bool operator==(const EventConsumer& rhs) {
			return (guid == rhs.guid);
		}

	  private:
		std::function<void(Event&)> consumer;
		xg::Guid guid;
	};
}