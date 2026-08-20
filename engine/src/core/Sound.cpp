#include "Cacao/Sound.hpp"
#include "Cacao/Exceptions.hpp"
#include "Cacao/AudioManager.hpp"
#include "impl/Sound.hpp"
#include "impl/ResourceManager.hpp"
#include "ImplAccessor.hpp"

#include "Bytestream.hpp"

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

	std::shared_ptr<Sound> Sound::Create(std::vector<char>&& encodedAudio, const std::string& addr) {
		std::shared_ptr<Sound> ptr(new Sound(std::move(encodedAudio), addr));
		std::lock_guard lk {IMPL(ResourceManager).cacheProtector};
		IMPL(ResourceManager).cache.insert_or_assign(addr, ptr);
		return ptr;
	}

	Sound::~Sound() {
		if(baked) Discard();
	}

	Sound::Sound(Sound&& other)
	  : Asset(other.address) {
		//Steal the implementation pointer
		impl = std::move(other.impl);

		//Copy baking state
		baked = other.baked;
		other.baked = false;

		//Blank out other asset address
		other.address = "";
	}

	Sound& Sound::operator=(Sound&& other) {
		//Implementation pointer
		impl = std::move(other.impl);

		//Baking state
		baked = other.baked;
		other.baked = false;

		//Asset address
		address = other.address;
		other.address = "";

		return *this;
	}

	void Sound::Bake() {
		Check<BadBakeStateException>(!baked, "Cannot bake a baked sound!");
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "The audio system must be initialized to bake a sound!");

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

		baked = true;
	}

	void Sound::Discard() {
		Check<BadBakeStateException>(baked, "Cannot discard baked representation of an unbaked sound; it does not exist!");
		Check<BadInitStateException>(AudioManager::Get().IsInitialized(), "The audio system must be initialized to drop a sound's baked representation!");

		baked = false;

		//Delete OpenAL buffer
		alDeleteBuffers(1, &impl->bufferObj);

		//Clear audio data struct
		impl->audio.channelCount = 0;
		impl->audio.sampleCount = 0;
		impl->audio.sampleRate = 0;
		impl->audio.data.clear();
	}
}