#include "Cacao/AudioPlayer.hpp"
#include "Cacao/AudioManager.hpp"
#include "Cacao/Exceptions.hpp"
#include "ImplAccessor.hpp"
#include "impl/Sound.hpp"
#include "impl/AudioPlayer.hpp"

#include "AL/al.h"
#include <memory>

namespace Cacao {
	AudioPlayer::AudioPlayer() {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot create an audio player when the audio system is not initialized!");

		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//Set up source object
		alGenSources(1, &impl->source);
		alSourcei(impl->source, AL_SOURCE_RELATIVE, AL_FALSE);
	}

	AudioPlayer::~AudioPlayer() {
		//Release sound
		SetSound({});

		//Delete source object
		alDeleteSources(1, &impl->source);
	}

	void AudioPlayer::Play() {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot start audio playback when the audio system is not initialized!");
		Check<Sound, NonexistentValueException>(sound, "Cannot play a null sound!");
		Check<BadBakeStateException>(sound->IsBaked(), "Cannot play an unbaked sound!");
		Check<BadStateException>(GetState() != State::Playing, "Cannot start playback when audio is already playing!");

		//Start playback
		alSourcePlay(impl->source);
	}

	void AudioPlayer::Pause() {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot pause audio playback when the audio system is not initialized!");
		Check<BadStateException>(GetState() == State::Playing, "Cannot start playback when audio is already playing!");

		//Pause playback
		alSourcePause(impl->source);
	}

	void AudioPlayer::Stop() {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot stop audio playback when the audio system is not initialized!");

		//Stop playback
		alSourceStop(impl->source);
	}

	void AudioPlayer::SetSound(std::shared_ptr<Sound> newSound) {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot set a sound for playback when the audio system is not initialized!");

		//Stop playback before doing anything
		Stop();

		//Set new sound
		sound = newSound;
		if(sound) {
			//Replace sound buffer
			alSourcei(impl->source, AL_BUFFER, IMPL(Sound, *sound).bufferObj);
		} else {
			//Detach the current buffer
			alSourcei(impl->source, AL_BUFFER, AL_NONE);
		}
	}

	AudioPlayer::State AudioPlayer::GetState() const {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot get playback state when the audio system is not initialized!");

		//Return appropriate state
		ALint state;
		alGetSourcei(impl->source, AL_SOURCE_STATE, &state);
		switch(state) {
			case AL_PLAYING:
				return State::Playing;
			case AL_PAUSED:
				return State::Paused;
			case AL_STOPPED:
			case AL_INITIAL:
			default:
				return State::Stopped;
		}
	}

	void AudioPlayer::SetLooping(bool val) {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot set audio player properties when the audio system is not initialized!");

		//Set property
		alSourcei(impl->source, AL_LOOPING, val);
	}

	void AudioPlayer::SetGain(float val) {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot set audio player properties when the audio system is not initialized!");
		Check<BadValueException>(val >= 0, "Cannot set gain value to a negative number!");

		//Set property
		alSourcef(impl->source, AL_GAIN, val);
	}

	void AudioPlayer::SetPitchMultiplier(float val) {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot set audio player properties when the audio system is not initialized!");
		Check<BadValueException>(val > 0, "Cannot set pitch multiplier value to a negative number or zero!");

		//Set property
		alSourcef(impl->source, AL_PITCH, val);
	}

	void AudioPlayer::SetPosition(float position) {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot set audio player properties when the audio system is not initialized!");
		Check<BadValueException>(position >= 0 && position <= ((float)IMPL(Sound, *sound).audio.sampleCount / IMPL(Sound, *sound).audio.sampleRate), "Cannot set audio playback position to an invalid position!");

		//Set property
		alSourcef(impl->source, AL_SEC_OFFSET, position);
	}

	bool AudioPlayer::GetLooping() const {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot get audio player properties when the audio system is not initialized!");

		//Query property
		ALint value;
		alGetSourcei(impl->source, AL_LOOPING, &value);
		return value == AL_TRUE;
	}

	float AudioPlayer::GetGain() const {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot get audio player properties when the audio system is not initialized!");

		//Query property
		ALfloat value;
		alGetSourcef(impl->source, AL_GAIN, &value);
		return value;
	}

	float AudioPlayer::GetPitchMultiplier() const {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot get audio player properties when the audio system is not initialized!");

		//Query property
		ALfloat value;
		alGetSourcef(impl->source, AL_PITCH, &value);
		return value;
	}

	float AudioPlayer::GetPosition() const {
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "Cannot get audio player properties when the audio system is not initialized!");

		//Query property
		ALfloat value;
		alGetSourcef(impl->source, AL_SEC_OFFSET, &value);
		return value;
	}

	void AudioPlayer::OnEnable() {
		if(autoplay && sound) Play();
	}

	void AudioPlayer::OnDisable() {
		Stop();
	}
}