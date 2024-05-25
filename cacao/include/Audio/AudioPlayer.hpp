#pragma once

#include "Sound.hpp"
#include "World/Component.hpp"
#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	class AudioPlayer final : public Component {
	  public:
		AudioPlayer();
		~AudioPlayer() {
			delete nativeData;
		}

		//Player properties
		//These values will NOT update while a sound is being played
		bool is3D;
		bool isLooping;
		float gain;

		//Play the sound
		void Play();

		//Stop playing
		void Stop();

		AssetHandle<Sound> sound;

	  private:
		NativeData* nativeData;
	};
}