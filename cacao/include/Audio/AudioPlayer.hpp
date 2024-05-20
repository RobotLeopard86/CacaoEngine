#pragma once

#include "Sound.hpp"
#include "World/Component.hpp"

namespace Cacao {
	class AudioPlayer final : public Component {
	  public:
		bool is3D;
		bool isLooping;
		float gain;

		AssetHandle<Sound> sound;

		AudioPlayer(const UUIDv4::UUID& owner)
		  : Component(owner) {}
	};
}