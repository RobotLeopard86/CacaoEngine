#pragma once

#include "Utilities/Asset.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Events/EventSystem.hpp"

#include "AL/al.h"

#include <memory>
#include <string>

namespace Cacao {
	//A sound or music to play
	class Sound final : public Asset {
	  public:
		//Create a sound from an audio file path
		Sound(std::string filePath);
		~Sound() final {
			if(compiled) Release();
		}

		//Compile sound to be used later
		//Will be automatically released at audio system shutdown
		std::shared_future<void> Compile() override;

		//Delete compiled data when no longer needed
		void Release() override;

		std::string GetType() override {
			return "SOUND";
		}

	  private:
		//Sound data
		std::string filePath;
		unsigned int sampleRate;
		unsigned long long sampleCount;
		std::vector<signed short> audioData;
		unsigned int channelCount;

		//OpenAL object
		ALuint buf;

		SignalEventConsumer* sec;

		//Sound data initialization methods
		void _InitFlac();
		void _InitMP3();
		void _InitWAV();
		void _InitVorbis();
		void _InitOpus();

		//Need to let the FLAC decoder (which has to be a class per the API) access our methods
		friend class FLACDecoder;

		//Need the audio player to be able to see our stuff
		friend class AudioPlayer;
	};
}