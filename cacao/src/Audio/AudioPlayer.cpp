#include "Audio/AudioPlayer.hpp"

#include "Audio/AudioController.hpp"

#include "AL/alext.h"

namespace Cacao {

	AudioPlayer::AudioPlayer() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to create an audio player!")

		//Create OpenAL source object
		alGenSources(1, &source);

		//Configure source object
		alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);

		//Register a release event
		sec = new SignalEventConsumer([this](Event& e, std::promise<void>& p) {
			alDeleteSources(1, &source);
			EventManager::GetInstance()->UnsubscribeConsumer("AudioShutdown", sec);
			delete sec;
			p.set_value();
		});
		EventManager::GetInstance()->SubscribeConsumer("AudioShutdown", sec);
	}

	AudioPlayer::~AudioPlayer() {
		if(AudioController::GetInstance()->IsAudioSystemInitialized()) {
			//Delete source object
			alDeleteSources(1, &source);

			EventManager::GetInstance()->UnsubscribeConsumer("AudioShutdown", sec);
			delete sec;
		}
	}

	bool AudioPlayer::IsPlaying() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get playback state!")

		ALenum isPlaying;
		alGetSourcei(source, AL_SOURCE_STATE, &isPlaying);
		return (isPlaying == AL_PLAYING);
	}

	bool AudioPlayer::IsPaused() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get pause state!")

		ALenum isPlaying;
		alGetSourcei(source, AL_SOURCE_STATE, &isPlaying);
		return (isPlaying == AL_PAUSED);
	}

	void AudioPlayer::Play() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to play sound!")
		CheckException(!IsPlaying(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot play a sound while playing one!")
		CheckException(!sound.IsNull(), Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot play null sound!")

		//Play the sound
		alSourcei(source, AL_BUFFER, sound->buf);
		alSourcePlay(source);
	}

	void AudioPlayer::TogglePause() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to pause sound playback!")
		CheckException(IsPlaying(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot pause a sound when not playing one!")

		if(IsPaused()) {
			//This will continue playback if paused (which it is)
			alSourcePlay(source);
		} else {
			alSourcePause(source);
		}
	}

	void AudioPlayer::Stop() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to stop sound playback!")
		CheckException(IsPlaying(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Cannot stop a sound when not playing one!")

		alSourceStop(source);
	}

	void AudioPlayer::Set3DSpatializationEnabled(bool val) {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to change 3D spatialization state!")

		alSourcei(source, AL_SOURCE_SPATIALIZE_SOFT, val ? AL_TRUE : AL_FALSE);
	}

	bool AudioPlayer::Get3DSpatializationEnabled() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get 3D spatialization state!")

		ALint retval;
		alGetSourcei(source, AL_SOURCE_SPATIALIZE_SOFT, &retval);
		return retval;
	}

	void AudioPlayer::SetLooping(bool val) {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to change looping state!")

		alSourcei(source, AL_LOOPING, val ? AL_TRUE : AL_FALSE);
	}

	bool AudioPlayer::GetLooping() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get looping state!")

		ALint retval;
		alGetSourcei(source, AL_LOOPING, &retval);
		return retval;
	}

	void AudioPlayer::SetGain(float val) {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to change gain!")

		alSourcef(source, AL_GAIN, val);
	}

	float AudioPlayer::GetGain() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get gain!")

		float retval;
		alGetSourcef(source, AL_GAIN, &retval);
		return retval;
	}

	void AudioPlayer::SetPitchMultiplier(float val) {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to change pitch multiplier!")

		alSourcef(source, AL_PITCH, val);
	}

	float AudioPlayer::GetPitchMultiplier() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get pitch multiplier!")

		float retval;
		alGetSourcef(source, AL_PITCH, &retval);
		return retval;
	}

	void AudioPlayer::SetPlaybackTime(float val) {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to change playback time!")

		alSourcef(source, AL_SEC_OFFSET, val);
	}

	float AudioPlayer::GetPlaybackTime() {
		CheckException(AudioController::GetInstance()->IsAudioSystemInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get playback time!")

		float retval;
		alGetSourcef(source, AL_SEC_OFFSET, &retval);
		return retval;
	}
}