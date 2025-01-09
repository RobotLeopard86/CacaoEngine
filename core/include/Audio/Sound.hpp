#pragma once

#include "Assets/Asset.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Events/EventSystem.hpp"
#include "Core/DllHelper.hpp"

#include <memory>
#include <string>

namespace Cacao {
	/**
	 * @brief A sound that can be played
	 */
	class CACAO_API Sound final : public Asset {
	  public:
		/**
		 * @brief Load a sound from a path
		 * @note Prefer to use AssetManager::LoadSound over direct construction
		 *
		 * @param pcmData The PCM audio data
		 * @param samples The number of audio samples in the data
		 * @param samplesRate Audio sampling rate
		 * @param channels Audio channel count
		 *
		 * @throws Exception If the file does not exist, could not be opened, is of an unsupported format, or its data could not be read
		 */
		Sound(std::vector<short> pcmData, unsigned long long samples, unsigned int sampleRate, unsigned int channels);
		~Sound() final {
			if(compiled) Release();
		}

		/**
		 * @brief Compile the raw sound data into a format that can be played asynchronously
		 *
		 * @return A future that will resolve when compilation is done
		 *
		 * @note The compiled data will be automatically released when at AudioSystem shuts down
		 *
		 * @throws Exception If the sound was already compiled or the audio system is uninitialized
		 */
		std::shared_future<void> CompileAsync() override;

		/**
		 * @brief Compile the raw sound data into a format that can be played synchronously
		 *
		 * @note The compiled data will be automatically released when at AudioSystem shuts down
		 *
		 * @throws Exception If the sound was already compiled or the audio system is uninitialized
		 */
		void CompileSync() override;

		/**
		 * @brief Delete the compiled data
		 *
		 * @throws Exception If the sound was not compiled or the audio system is uninitialized
		 */
		void Release() override;

		///@brief Gets the type of this asset. Needed for safe downcasting from Asset
		std::string GetType() const override {
			return "SOUND";
		}

	  private:
		//Sound data
		unsigned int sampleRate;
		unsigned long long sampleCount;
		std::vector<short> audioData;
		unsigned int channelCount;

		//OpenAL object
		unsigned int buf;

		SignalEventConsumer* sec;

		//Need the audio player to be able to see our stuff
		friend class AudioPlayer;
	};
}