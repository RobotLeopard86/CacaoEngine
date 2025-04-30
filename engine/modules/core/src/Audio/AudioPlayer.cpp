#include "Audio/AudioPlayer.hpp"

#include "Audio/AudioSystem.hpp"

#include "AL/alext.h"

namespace Cacao {

	AudioPlayer::AudioPlayer() {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to create an audio player!");

		//Create OpenAL source object
		alGenSources(1, &source);

		//Configure source object
		alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);

		//Register a release event
		sec = new SignalEventConsumer([this](Event& e, std::promise<void>& p) {
			//Stop playing and unlink buffer
			if(IsPlaying()) Stop();
			alSourcei(source, AL_BUFFER, AL_NONE);

			//Delete source object
			alDeleteSources(1, &source);

			//Remove consumers
			if(consumersSubscribed) {
				EventManager::GetInstance()->UnsubscribeConsumer("AudioShutdown", sec);
				EventManager::GetInstance()->UnsubscribeConsumer("SoundRelease", soundDelete);
				delete sec;
				delete soundDelete;
				consumersSubscribed = false;
			}

			//Notify that we're done
			p.set_value();
		});
		EventManager::GetInstance()->SubscribeConsumer("AudioShutdown", sec);

		//Register an event for sound releasing
		soundDelete = new SignalEventConsumer([this](Event& e, std::promise<void>& p) {
			DataEvent<ALuint>& de = static_cast<DataEvent<ALuint>&>(e);

			//Check if this sound is ours, if it is we have to stop playback
			if(!sound.IsNull() && de.GetData() == sound->buf) {
				//Stop player and unlink buffer
				if(IsPlaying()) Stop();
				alSourcei(source, AL_BUFFER, AL_NONE);
			}

			//Notify that we're done
			p.set_value();
		});
		EventManager::GetInstance()->SubscribeConsumer("SoundRelease", soundDelete);

		consumersSubscribed = true;
	}

	AudioPlayer::~AudioPlayer() {
		if(AudioSystem::GetInstance()->IsInitialized()) {
			//Stop playing and unlink buffer
			if(IsPlaying()) Stop();
			alSourcei(source, AL_BUFFER, AL_NONE);

			//Delete source object
			alDeleteSources(1, &source);
		}

		if(consumersSubscribed) {
			//Remove consumers
			EventManager::GetInstance()->UnsubscribeConsumer("AudioShutdown", sec);
			EventManager::GetInstance()->UnsubscribeConsumer("SoundRelease", soundDelete);
			delete sec;
			delete soundDelete;
		}
	}

	bool AudioPlayer::IsPlaying() {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get playback state!");

		ALenum isPlaying;
		alGetSourcei(source, AL_SOURCE_STATE, &isPlaying);
		return (isPlaying == AL_PLAYING);
	}

	bool AudioPlayer::IsPaused() {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get pause state!");

		ALenum isPlaying;
		alGetSourcei(source, AL_SOURCE_STATE, &isPlaying);
		return (isPlaying == AL_PAUSED);
	}

	void AudioPlayer::Play() {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to play sound!");
		CheckException(!IsPlaying(), Exception::GetExceptionCodeFromMeaning("BadState"), "Cannot play a sound while playing one!");
		CheckException(!sound.IsNull(), Exception::GetExceptionCodeFromMeaning("NullValue"), "Cannot play null sound!");

		//Play the sound
		alSourcei(source, AL_BUFFER, sound->buf);
		alSourcePlay(source);
	}

	void AudioPlayer::TogglePause() {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to pause sound playback!");
		CheckException(IsPlaying() || IsPaused(), Exception::GetExceptionCodeFromMeaning("BadState"), "Cannot pause a sound when not playing one!");

		if(IsPaused()) {
			//This will continue playback if paused (which it is)
			alSourcePlay(source);
		} else {
			alSourcePause(source);
		}
	}

	void AudioPlayer::Stop() {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to stop sound playback!");
		CheckException(IsPlaying() || IsPaused(), Exception::GetExceptionCodeFromMeaning("BadState"), "Cannot stop a sound when not playing one!");

		alSourceStop(source);
		alSourcei(source, AL_BUFFER, AL_NONE);
	}

	void AudioPlayer::SetLooping(bool val) {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to change looping state!");

		alSourcei(source, AL_LOOPING, val ? AL_TRUE : AL_FALSE);
	}

	bool AudioPlayer::GetLooping() {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get looping state!");

		ALint retval;
		alGetSourcei(source, AL_LOOPING, &retval);
		return retval;
	}

	void AudioPlayer::SetGain(float val) {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to change gain!");

		alSourcef(source, AL_GAIN, val);
	}

	float AudioPlayer::GetGain() {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get gain!");

		float retval;
		alGetSourcef(source, AL_GAIN, &retval);
		return retval;
	}

	void AudioPlayer::SetPitchMultiplier(float val) {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to change pitch multiplier!");

		alSourcef(source, AL_PITCH, val);
	}

	float AudioPlayer::GetPitchMultiplier() {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get pitch multiplier!");

		float retval;
		alGetSourcef(source, AL_PITCH, &retval);
		return retval;
	}

	void AudioPlayer::SetPlaybackTime(float val) {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to change playback time!");

		alSourcef(source, AL_SEC_OFFSET, val);
	}

	float AudioPlayer::GetPlaybackTime() {
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to get playback time!");

		float retval;
		alGetSourcef(source, AL_SEC_OFFSET, &retval);
		return retval;
	}
}