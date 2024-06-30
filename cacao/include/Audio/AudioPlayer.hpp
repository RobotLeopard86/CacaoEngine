#pragma once

#include "Sound.hpp"
#include "World/Component.hpp"
#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	class AudioPlayer final : public Component {
	  public:
		AudioPlayer();
		~AudioPlayer();

		//Play the sound
		void Play();

		//Toggle paused playback
		void TogglePause();

		//Stop playing
		void Stop();

		//Check if playing
		bool IsPlaying();

		//Check if paused
		bool IsPaused();

		//Sound to play
		AssetHandle<Sound> sound;

		void SetLooping(bool val);
		void SetGain(float val);
		void SetPitchMultiplier(float val);
		void SetPlaybackTime(float timeInSeconds);
		bool GetLooping();
		float GetGain();
		float GetPitchMultiplier();
		float GetPlaybackTime();

	  private:
		//OpenAL object
		ALuint source;

		SignalEventConsumer* sec;
		SignalEventConsumer* soundDelete;
		bool consumersSubscribed;
	};
}