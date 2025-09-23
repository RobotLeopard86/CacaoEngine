#pragma once

#include "Cacao/Sound.hpp"

#include "AL/al.h"

#include "libcacaoaudiodecode.hpp"

namespace Cacao {
	struct Sound::Impl {
		//Encoded audio buffer
		std::vector<char> encodedAudio;

		//Decoded audio and OpenAL object
		ALuint bufferObj;
		libcacaoaudiodecode::Result audio;
	};
}