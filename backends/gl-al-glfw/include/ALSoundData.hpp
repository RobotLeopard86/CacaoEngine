#pragma once

#include "Utilities/MiscUtils.hpp"
#include "Events/EventSystem.hpp"

#include "alure2.h"

namespace Cacao {
	//Struct for data required by an OpenAL sound
	struct ALSoundData : public NativeData {
		alure::Buffer data;
		SignalEventConsumer* consumer;
		bool didRegisterConsumer;

		//If the consumer was registered, unregister and free it
		void TryDeleteConsumer() {
			if(didRegisterConsumer) {
				EventManager::GetInstance()->UnsubscribeConsumer("AudioContextDestruction", consumer);
				delete consumer;
				didRegisterConsumer = false;
			}
		}

		~ALSoundData() {
			TryDeleteConsumer();
		}
	};
}