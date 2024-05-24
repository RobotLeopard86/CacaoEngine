#pragma once

#include "Utilities/Asset.hpp"
#include "Utilities/MiscUtils.hpp"

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
			delete nativeData;
		}

		//Compile sound to be used later
		std::shared_future<void> Compile() override;
		//Delete sound when no longer needed
		void Release() override;

		std::string GetType() override {
			return "SOUND";
		}

		//Access the native data
		//This should ONLY be used by audio players
		//Also you have to solemnly swear that you won't delete it (this is definitely a good safety tactic)
		NativeData* GetNativeData(bool iSolemnlySwearThatIWillNotDeleteThisPointer) {
			if(!iSolemnlySwearThatIWillNotDeleteThisPointer) return nullptr;
			return nativeData;
		}

	  private:
		std::string path;
		NativeData* nativeData;
	};
}