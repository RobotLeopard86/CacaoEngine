#include "ALSoundData.hpp"
#include "ALPlayerData.hpp"
#include "AudioObjectHolder.hpp"
#include "Audio/AudioController.hpp"
#include "Audio/Sound.hpp"
#include "Audio/AudioPlayer.hpp"
#include "Core/Exception.hpp"
#include "Core/Engine.hpp"
#include "Events/EventSystem.hpp"

#include "alure2.h"

#include <filesystem>

//For my sanity
#define nd ((ALPlayerData*)this->nativeData)
#define s_nd(s_hndl) ((ALSoundData*)s_hndl->GetNativeData(true))

namespace Cacao {
	AudioPlayer::AudioPlayer() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to create audio source!");

		//Create native data
		nativeData = new ALPlayerData();

		//Create audio source object
		nd->src = audioCtx.createSource();

		//Register releaser on audio context destruction
		nd->consumer = new SignalEventConsumer([this](Event& e, std::promise<void>& p) {
			//Release data
			nd->TryDelete();

			p.set_value();
		});
		nd->isSetup = true;
	}

	void AudioPlayer::Play() {
		CheckException(!sound.IsNull(), Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot play sound that doesn't exist!")
		CheckException(!nd->src.isPlaying(), Exception::GetExceptionCodeFromMeaning("AudioError"), "Cannot play sound from player that is already playing a sound!")

		//Configure source properties
		nd->src.setGain(gain);
		nd->src.set3DSpatialize(is3D ? alure::Spatialize::On : alure::Spatialize::Off);
		nd->src.setLooping(isLooping);

		//Start playing sound buffer
		nd->src.play(s_nd(sound)->data);
	}

	void AudioPlayer::Stop() {
		CheckException(nd->src.isPlaying(), Exception::GetExceptionCodeFromMeaning("AudioError"), "Cannot stop sound of player that is not playing a sound!")

		//Stop playing sound
		nd->src.stop();
	}
}