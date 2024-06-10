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

		//Play the sound
		void Play();

		//Stop playing
		void Stop();

		//Check if playing
		bool IsPlaying() {
			RefreshPlayState();
			return isPlaying;
		}

		AssetHandle<Sound> sound;

		bool Is3DSpatializationEnabled() {
			return is3D;
		}
		bool IsLooping() {
			return isLooping;
		}
		float GetGain() {
			return gain;
		}

		void Set3DSpatializationEnabled(bool val) {
			is3D = val;
			On3DChange();
		}
		void SetLooping(bool val) {
			isLooping = val;
			OnLoopChange();
		}
		void SetGain(float val) {
			gain = val;
			OnGainChange();
		}

	  private:
		NativeData* nativeData;

		//Player properties
		bool is3D;
		bool isLooping;
		float gain;

		//Backend-specific functions
		void On3DChange();
		void OnLoopChange();
		void OnGainChange();
		void RefreshPlayState();

		bool isPlaying;
	};
}