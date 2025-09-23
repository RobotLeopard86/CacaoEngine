#pragma once

#include "DllHelper.hpp"

#include <string>

namespace Cacao {
	/**
	 * @brief An event
	 */
	class CACAO_API Event {
	  public:
		/**
		 * @brief Create a new event
		 *
		 * @param eventType The type of event; see the page "Events" in the manual for a list of engine events
		 */
		Event(std::string eventType) {
			type = eventType;
		}

		/**
		 * @brief Get the event type
		 *
		 * @return The event type
		 */
		const std::string GetType() const {
			return type;
		}

	  protected:
		std::string type;
	};

	/**
	 * @brief An event that stores some data
	 */
	template<typename T>
		requires std::is_trivially_copy_constructible_v<T>
	class CACAO_API DataEvent : public Event {
	  public:
		/**
		 * @brief Create a new data event
		 *
		 * @param eventType The type of event; see the page "Events" in the manual for a list of engine events and their data types
		 *
		 * @param eventData The data to store
		 */
		DataEvent(std::string eventType, T eventData)
		  : Event(eventType), data(eventData) {}

		/**
		 * @brief Get the stored data
		 *
		 * @return The data
		 */
		T GetData() const {
			return data;
		}

	  private:
		T data;
	};
}