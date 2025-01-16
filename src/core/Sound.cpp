#include "Audio/Sound.hpp"

#include "Core/Exception.hpp"
#include "Core/Engine.hpp"
#include "Audio/AudioSystem.hpp"

#include "AL/al.h"

#include <filesystem>
#include <fstream>

#define RETHROW_EXCEPTION(x)                              \
	try {                                                 \
		x                                                 \
	} catch(...) {                                        \
		std::rethrow_exception(std::current_exception()); \
	}


namespace Cacao {
	Sound::Sound(std::string filePath)
	  : Asset(false), filePath(filePath) {
		CheckException(std::filesystem::exists(filePath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot load sound from nonexistent file!");

	}

	std::shared_future<void> Sound::CompileAsync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled sound!");
		return Engine::GetInstance()->GetThreadPool()->enqueue([this]() { this->CompileSync(); }).share();
	}

	void Sound::CompileSync() {
		CheckException(!compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled sound!");
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to compile a sound!");

		//Create buffer object
		alGenBuffers(1, &buf);

		//Load buffer with audio data
		alBufferData(buf, channelCount == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, audioData.data(), audioData.size() * sizeof(short), sampleRate);

		//Register a release event
		sec = new SignalEventConsumer([this](Event& e, std::promise<void>& p) {
			this->Release();
			p.set_value();
		});
		EventManager::GetInstance()->SubscribeConsumer("AudioShutdown", sec);

		compiled = true;
	}

	void Sound::Release() {
		CheckException(compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled sound!");
		CheckException(AudioSystem::GetInstance()->IsInitialized(), Exception::GetExceptionCodeFromMeaning("BadInitState"), "Audio system must be initialized to compile a sound!");

		//Send out an event to let all players using this sound stop
		DataEvent<ALuint> iAmBeingReleased("SoundRelease", buf);
		EventManager::GetInstance()->DispatchSignaled(iAmBeingReleased)->WaitAll();

		//Delete buffer object
		alDeleteBuffers(1, &buf);

		//Unregister release event
		EventManager::GetInstance()->UnsubscribeConsumer("AudioShutdown", sec);
		delete sec;

		compiled = false;
	}
}