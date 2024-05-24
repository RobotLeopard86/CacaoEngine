#pragma once

#include "Sound.hpp"
#include "World/Component.hpp"

namespace Cacao {
	class AudioPlayer final : public Component {
	  public:
		bool is3D;
		bool isLooping;
		float gain;

		//Play the sound
		void Play();

		AssetHandle<Sound> sound;
	};
}