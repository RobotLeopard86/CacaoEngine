#pragma once

#include "Utilities/Asset.hpp"
#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	//A sound or music to play
	class Sound : public Asset {
	  public:
		//Create a sound from an audio file path
		Sound(std::string filePath);

		//Compile sound to be used later
		std::shared_future<void> Compile() override;
		//Delete sound when no longer needed
		void Release() override;
		//Play the sound
		void Play();

		std::string GetType() override {
			return "SOUND";
		}

	  private:
		NativeData* nativeData;
	};
}