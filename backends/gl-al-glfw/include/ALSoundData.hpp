#pragma once

#include "Utilities/MiscUtils.hpp"
#include "Events/EventSystem.hpp"

#include "alure2.h"

namespace Cacao {
	//Struct for data required by an OpenAL sound
	struct ALSoundData : public NativeData {
		alure::Buffer data;
		SignalEventConsumer* releaseConsumer;
	};
}