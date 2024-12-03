#pragma once

#include "Utilities/Asset.hpp"
#include "Utilities/MiscUtils.hpp"
#include "Events/EventSystem.hpp"

#include "AL/al.h"

#include <memory>
#include <string>

namespace Cacao {
	/**
	 * @brief A sound that can be played
	 */
	class Sound final : public Asset {
	  public:
		/**
		 * @brief Load a sound from a path
		 * @details Supports MP3, WAV, Ogg Vorbis, and Ogg Opus
		 * @note Prefer to use AssetManager::LoadSound over direct construction
		 *
		 * @param filePath The path to load from
		 *
		 * @throws Exception If the file does not exist, could not be opened, is of an unsupported format, or its data could not be read
		 */
		Sound(std::string filePath);
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
		std::string GetType() override {
			return "SOUND";
		}

	  private:
		//Sound data
		std::string filePath;
		unsigned int sampleRate;
		unsigned long long sampleCount;
		std::vector<short> audioData;
		unsigned int channelCount;

		//OpenAL object
		ALuint buf;

		SignalEventConsumer* sec;

		//Sound data initialization methods
		void _InitFlac();
		void _InitMP3();
		void _InitWAV();
		void _InitVorbis();
		void _InitOpus();

		//Need to let the FLAC decoder (which has to be a class per the API) access our methods
		friend class FLACDecoder;

		//Need the audio player to be able to see our stuff
		friend class AudioPlayer;
	};
}