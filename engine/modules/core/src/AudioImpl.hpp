#pragma once

#include "Cacao/Sound.hpp"

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#include "libcacaoaudiodecode.hpp"

namespace Cacao {
	struct Sound::Impl {
		ALuint bufferObj;
		libcacaoaudiodecode::Result audio;
	};
}