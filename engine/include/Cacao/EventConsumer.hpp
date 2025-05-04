#pragma once

#include "Event.hpp"
#include "Exceptions.hpp"
#include "MultiFuture.hpp"
#include "DllHelper.hpp"

#include <functional>
#include <vector>

namespace Cacao {
	/**
	 * @brief Simple wrapper for a function that consumes an event
	 */
	class CACAO_API EventConsumer {
	  public:
		/**
		 * @brief Create an event consumer
		 *
		 * @param consumer The consuming function
		 */
		EventConsumer(std::function<void(Event&)> consumer)
		  : consumer(consumer) {}

		/**
		 * @brief Consume an event
		 * @details Forwards the event to the consuming function
		 */
		void Consume(Event& event) {
			consumer(event);
		}

		/**
		 * @brief Check if two EventConsumers are equal
		 * @details Compares memory addresses to perform check
		 */
		bool operator==(EventConsumer rhs) {
			return (this == &rhs);
		}

	  private:
		std::function<void(Event&)> consumer;
	};
}