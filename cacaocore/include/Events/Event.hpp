#pragma once

#include "Core/Log.hpp"
#include "Core/Assert.hpp"

#include <string>
#include <optional>

namespace Cacao {
	//Event class
	class Event {
	public:
		Event(std::string eventType) {
			type = eventType;
		}

        std::string GetType() { return type; }
	protected:
		std::string type;
	};

	//Event class that holds data
	template<typename T>
    class DataEvent : public Event {
    public:
		DataEvent(std::string eventType, T eventData) 
			: Event(eventType) {
			data = std::make_optional<T>(eventData);
		}

		bool HasData() { return data.has_value(); }
		T GetData() {
			Asserts::EngineAssert(HasData(), "Cannot access event data which does not exist!");
			return data.value();
		}
	private:
		std::optional<T> data;
    };
}