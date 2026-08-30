#pragma once

#include "Cacao/AudioPlayer.hpp"

#include "AL/al.h"

namespace Cacao {
	struct AudioPlayer::Impl {
		//AL source
		ALuint source;
	};
}