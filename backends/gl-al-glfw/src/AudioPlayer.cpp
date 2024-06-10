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
	AudioPlayer::AudioPlayer()
	  : sound(), is3D(false), isLooping(false), gain(1.0f), isPlaying(false) {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to create audio source!");

		//Create native data
		nativeData = new ALPlayerData();

		//Run audio-specific code on audio thread
		std::shared_future<void> setupJob = AudioController::GetInstance()->RunAudioThreadJob([this]() {
			//Create audio source object
			alure::Context::MakeCurrent(audioCtx);
			nd->src = audioCtx.createSource();
			nd->src.setRelative(true);

			//Register releaser on audio context destruction
			nd->consumer = new SignalEventConsumer([this](Event& e, std::promise<void>& p) {
				//Release data
				nd->TryDelete();

				p.set_value();
			});
			nd->isSetup = true;
		});
		setupJob.wait();
	}

	void AudioPlayer::Play() {
		CheckException(!sound.IsNull(), Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot play sound that doesn't exist!")
		CheckException(!isPlaying, Exception::GetExceptionCodeFromMeaning("AudioError"), "Cannot play sound from player that is already playing a sound!")

		std::shared_future<void> playJob = AudioController::GetInstance()->RunAudioThreadJob([this]() {
			//Start playing sound buffer
			nd->src.play(s_nd(sound)->data);

			isPlaying = true;
		});
		playJob.wait();
	}

	void AudioPlayer::Stop() {
		CheckException(isPlaying, Exception::GetExceptionCodeFromMeaning("AudioError"), "Cannot stop sound of player that is not playing a sound!")

		std::shared_future<void> stopJob = AudioController::GetInstance()->RunAudioThreadJob([this]() {
			//Start playing sound buffer
			nd->src.stop();

			isPlaying = true;
		});
		stopJob.wait();
	}

	void AudioPlayer::RefreshPlayState() {
		std::shared_future<void> refreshJob = AudioController::GetInstance()->RunAudioThreadJob([this]() {
			this->isPlaying = nd->src.isPlaying();
		});
		refreshJob.wait();
	}

	void AudioPlayer::On3DChange() {
		std::shared_future<void> changeJob = AudioController::GetInstance()->RunAudioThreadJob([this]() {
			nd->src.set3DSpatialize(this->is3D ? alure::Spatialize::On : alure::Spatialize::Off);
		});
		changeJob.wait();
	}

	void AudioPlayer::OnLoopChange() {
		std::shared_future<void> changeJob = AudioController::GetInstance()->RunAudioThreadJob([this]() {
			nd->src.setLooping(this->isLooping);
		});
		changeJob.wait();
	}

	void AudioPlayer::OnGainChange() {
		std::shared_future<void> changeJob = AudioController::GetInstance()->RunAudioThreadJob([this]() {
			nd->src.setGain(this->gain);
		});
		changeJob.wait();
	}
}