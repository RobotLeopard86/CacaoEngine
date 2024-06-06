#pragma once

#include "Utilities/MiscUtils.hpp"

#include "alure2.h"

namespace Cacao {
	//Struct for data required by an OpenAL sound player
	struct ALPlayerData : public NativeData {
		alure::Source src;
		SignalEventConsumer* consumer;
		bool isSetup;

		//If the consumer was registered, unregister and free it
		void TryDelete() {
			if(isSetup) {
				EventManager::GetInstance()->UnsubscribeConsumer("AudioContextDestruction", consumer);
				delete consumer;
				src.destroy();
				isSetup = false;
			}
		}

		~ALPlayerData() {
			TryDelete();
		}
	};
}