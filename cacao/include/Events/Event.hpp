#pragma once

#include "Core/Log.hpp"
#include "Core/Assert.hpp"

#include <string>

namespace Cacao {
	//Event class
	class Event {
	  public:
		Event(std::string eventType) {
			type = eventType;
		}

		std::string GetType() {
			return type;
		}

	  protected:
		std::string type;
	};

	//Event class that holds data
	template<typename T>
	class DataEvent : public Event {
	  public:
		DataEvent(std::string eventType, T eventData)
		  : Event(eventType), data(eventData) {
			static_assert(std::is_trivially_copy_constructible_v<T>, "Type used in data event must be trivally copy-contructible!");
		}

		T GetData() {
			return data;
		}

	  private:
		T data;
	};
}