#pragma once

#include "Event.hpp"
#include "Core/Exception.hpp"
#include "Utilities/MultiFuture.hpp"

#include <functional>
#include <vector>

namespace Cacao {
	/**
	 * @brief Simple wrapper for a function that consumes an event
	 */
	class EventConsumer {
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
		virtual void Consume(Event& event) {
			consumer(event);
		}

		/**
		 * @brief Check if two EventConsumers are equal
		 * @details Compares memory addresses to check
		 */
		bool operator==(EventConsumer rhs) {
			return (this == &rhs);
		}

		//Virtual destructor
		virtual ~EventConsumer() {}

	  protected:
		//Here so the signal event consumer can initialize itself with a different consumer type
		EventConsumer() {}

	  private:
		std::function<void(Event&)> consumer;
	};

	/**
	 * @brief An extended event consumer that can signal when it's done via a promise
	 */
	class SignalEventConsumer : public EventConsumer {
	  public:
		/**
		 * @brief Create a new signal event consumer
		 *
		 * @param consumer The consuming function
		 */
		SignalEventConsumer(std::function<void(Event&, std::promise<void>&)> consumer)
		  : EventConsumer(), consumer(consumer) {}

		/**
		 * @brief Consume an event with a signal
		 * @details Forwards the event and a promise to the consuming function, which should set the promise value to indicate that it's done
		 */
		void ConsumeWithSignal(Event& event, MultiFuture<void>& signal) {
			std::promise<void> signalPromise = std::promise<void>();
			signal.push_back(signalPromise.get_future());
			consumer(event, signalPromise);
		}

		/**
		 * @brief (DISALLOWED) Consume an unsignaled event
		 * @warning This function does nothing, because signal event consumers require a signal
		 * @throws Exception When called
		 */
		void Consume(Event& event) override final {
			CheckException(false, Exception::GetExceptionCodeFromMeaning("EventManager"), "Signal event consumers cannot consume events without signals!")
		}

	  private:
		std::function<void(Event&, std::promise<void>&)> consumer;
	};
}