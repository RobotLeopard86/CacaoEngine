#include "Cacao/AudioManager.hpp"
#include "Cacao/Exceptions.hpp"

#include "AL/al.h"
#include "AL/alc.h"

#include <functional>
#include <memory>

namespace Cacao {
	struct AudioManager::Impl {
		std::unique_ptr<ALCdevice, std::function<void(ALCdevice*)>> dev;
		std::unique_ptr<ALCcontext, std::function<void(ALCcontext*)>> ctx;
		bool init;
	};

	AudioManager::AudioManager() {
		//Create implementation pointer
		impl = std::make_unique<Impl>();
		impl->init = false;
	}

	AudioManager::~AudioManager() {
		if(IsInitialized()) Terminate();
	}

	void AudioManager::Initialize() {
		Check<BadInitStateException>(!IsInitialized(), "The audio system must not be initialized when Initialize is called!");

		//Create ALC device
		impl->dev = std::unique_ptr<ALCdevice, std::function<void(ALCdevice*)>>(alcOpenDevice(nullptr), alcCloseDevice);
		Check<ExternalException>((bool)impl->dev, "Failed to create OpenAL device!");

		//Create ALC context
		impl->ctx = std::unique_ptr<ALCcontext, std::function<void(ALCcontext*)>>(alcCreateContext(impl->dev.get(), nullptr), alcDestroyContext);
		Check<ExternalException>((bool)impl->ctx, "Failed to create OpenAL context!");

		//Activate context
		alcMakeContextCurrent(impl->ctx.get());

		//Mark as initialized
		impl->init = true;

		//Set initial gain
		SetGlobalGain(1.0f);
	}

	void AudioManager::Terminate() {
		Check<BadInitStateException>(IsInitialized(), "The audio system must be initialized when Terminate is called!");

		//We are no longer initialized
		impl->init = false;

		//Destroy context and device (this will invoke the deleter function and deal with ALC for us)
		impl->ctx.reset(nullptr);
		impl->dev.reset(nullptr);
	}

	bool AudioManager::IsInitialized() {
		return impl->init;
	}

	void AudioManager::SetGlobalGain(float value) {
		Check<BadInitStateException>(IsInitialized(), "The audio system must be initialized to set the global gain is called!");

		alListenerf(AL_GAIN, value);
	}

	float AudioManager::GetGlobalGain() {
		Check<BadInitStateException>(IsInitialized(), "The audio system must be initialized to check the global gain is called!");

		float retval;
		alGetListenerf(AL_GAIN, &retval);
		return retval;
	}
}