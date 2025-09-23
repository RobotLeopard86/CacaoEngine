#include "Cacao/Sound.hpp"
#include "Cacao/ThreadPool.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/AudioManager.hpp"
#include "impl/Sound.hpp"

#include "libcacaocommon.hpp"

namespace Cacao {
	Sound::Sound(std::vector<char>&& encodedAudio, const std::string& addr)
	  : Asset(addr) {
		Check<BadValueException>(ValidateResourceAddr<Sound>(addr), "Resource address is malformed!");
		Check<BadValueException>(!encodedAudio.empty(), "Cannot construct a sound with an empty audio buffer!");

		//Create implementation pointer
		impl = std::make_unique<Impl>();

		//Move audio buffer
		impl->encodedAudio = encodedAudio;
	}

	Sound::~Sound() {
		if(realized) DropRealized();
	}

	Sound::Sound(Sound&& other)
	  : Asset(other.address) {
		//Steal the implementation pointer
		impl = std::move(other.impl);

		//Copy realization state
		realized = other.realized;
		other.realized = false;

		//Blank out other asset address
		other.address = "";
	}

	Sound& Sound::operator=(Sound&& other) {
		//Implementation pointer
		impl = std::move(other.impl);

		//Realization state
		realized = other.realized;
		other.realized = false;

		//Asset address
		address = other.address;
		other.address = "";

		return *this;
	}

	void Sound::Realize() {
		Check<BadRealizeStateException>(!realized, "Sound must not be realized when Realize is called!");
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "The audio system must be initialized to realize a sound!");

		//Decode audio (yes, we rethrow the exception. deal with it.)
		ibytestream audioIn(impl->encodedAudio);
		try {
			impl->audio = libcacaoaudiodecode::DecodeAudio(audioIn);
		} catch(const std::runtime_error& e) {
			std::stringstream s;
			s << "Audio decoding failed! Decoder returned message \"" << e.what() << "\"";
			Check<ExternalException>(false, s.str());
		}

		//Create OpenAL buffer
		alGenBuffers(1, &impl->bufferObj);
		Check<ExternalException>(alGetError() == AL_NO_ERROR, "Failed to create OpenAL buffer for sound!");
		alBufferData(impl->bufferObj, impl->audio.channelCount == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, impl->audio.data.data(), impl->audio.data.size() * sizeof(short), impl->audio.sampleRate);
		Check<ExternalException>(alGetError() == AL_NO_ERROR, "Failed to load data into OpenAL buffer!");

		realized = true;
	}

	std::shared_future<void> Sound::RealizeAsync() {
		Check<BadRealizeStateException>(!realized, "Sound must not be realized when RealizeAsync is called!");
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "The audio system must be initialized to realize a sound!");

		//Put task in pool
		return ThreadPool::Get().Exec(std::bind(&Sound::Realize, this));
	}

	void Sound::DropRealized() {
		Check<BadRealizeStateException>(realized, "Sound must be realized when DropRealize is called!");
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "The audio system must be initialized to drop a sound's realized representation!");

		realized = false;

		//Delete OpenAL buffer
		alDeleteBuffers(1, &impl->bufferObj);

		//Clear audio data struct
		impl->audio.channelCount = 0;
		impl->audio.sampleCount = 0;
		impl->audio.sampleRate = 0;
		impl->audio.data.clear();
	}
}