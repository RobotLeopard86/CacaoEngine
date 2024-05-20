#pragma once

#include "Event.hpp"
#include "Core/Exception.hpp"

#include "BS_thread_pool.hpp"

#include <functional>
#include <vector>

namespace Cacao {
	//Wraps a function consumer
	class EventConsumer {
	  public:
		EventConsumer(std::function<void(Event&)> consumer)
		  : consumer(consumer) {}

		virtual void Consume(Event& event) {
			consumer(event);
		}

		bool operator==(EventConsumer rhs) {
			return (this == &rhs);
		}

		//Virtual destructor
		virtual ~EventConsumer() {}

	  protected:
		std::function<void(Event&)> consumer;
	};

	using EventSignal = BS::multi_future<void>;

	//Extension to an event consumer that can work with signals
	class SignalEventConsumer : public EventConsumer {
	  public:
		SignalEventConsumer(std::function<void(Event&)> consumer)
		  : EventConsumer(consumer) {}

		void ConsumeWithSignal(Event& event, EventSignal& signal) {
			signalPromise = std::promise<void>();
			signal.push_back(signalPromise.get_future());
			consumer(event);
		}

		//Make the default consume function throw an exception because this kind requires the signal method to be used
		void Consume(Event& event) override final {
			CheckException(false, Exception::GetExceptionCodeFromMeaning("EventManager"), "Signal event consumers cannot consume events without signals!")
		}

	  private:
		std::promise<void> signalPromise;
	};
}