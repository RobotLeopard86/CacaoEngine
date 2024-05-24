#include "ALSoundData.hpp"
#include "ALPlayerData.hpp"
#include "AudioObjectHolder.hpp"
#include "Audio/AudioController.hpp"
#include "Audio/Sound.hpp"
#include "Audio/AudioPlayer.hpp"
#include "Core/Exception.hpp"
#include "Core/Engine.hpp"
#include "Events/EventSystem.hpp"

#include <filesystem>

//For my sanity
#define nd ((ALSoundData*)this->nativeData)

namespace Cacao {

	Sound::Sound(std::string filePath)
	  : Asset(false) {
		CheckException(std::filesystem::exists(filePath), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot create sound object from nonexistent file!");
		path = filePath;

		//Create native data
		nativeData = new ALSoundData();
	}

	std::shared_future<void> Sound::Compile() {
		std::future<void> fut = Engine::GetInstance()->GetThreadPool().submit_task([this]() {
			CheckException(std::filesystem::exists(this->path), Exception::GetExceptionCodeFromMeaning("FileNotFound"), "Cannot compile sound object that references a nonexistent file!");
			CheckException(!this->compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot compile compiled sound!");

			//Create sound buffer
			nd->data = audioCtx.getBuffer(this->path);

			//Register releaser on audio context destruction
			nd->consumer = new SignalEventConsumer([this](Event& e, std::promise<void>& p) {
				//Release data
				this->Release();

				p.set_value();
			});
			nd->didRegisterConsumer = true;

			this->compiled = true;
		});
		return fut.share();
	}

	void Sound::Release() {
		CheckException(this->compiled, Exception::GetExceptionCodeFromMeaning("BadCompileState"), "Cannot release uncompiled sound!");

		//Unregister audio context destruction consumer
		nd->TryDeleteConsumer();

		this->compiled = false;
	}
}