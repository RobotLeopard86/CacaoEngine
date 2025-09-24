#pragma once

#include "DllHelper.hpp"
#include "Asset.hpp"

#include <memory>
#include <vector>

namespace Cacao {
	/**
	 * @brief Asset type for audio
	 */
	class CACAO_API Sound final : public Asset {
	  public:
		/**
		 * @brief Create a new sound from encoded audio data
		 *
		 * @param encodedAudio A buffer of audio, encoded in the format of WAV, MP3, Ogg Vorbis, or Ogg Opus
		 * @param addr The resource address to associate with the sound
		 *
		 * @throws BadValueException If the audio buffer is empty
		 * @throws BadValueException If the address is malformed
		 */
		static std::shared_ptr<Sound> Create(std::vector<char>&& encodedAudio, const std::string& addr) {
			return std::shared_ptr<Sound>(new Sound(std::move(encodedAudio), addr));
		}

		///@cond
		Sound(const Sound&) = delete;
		Sound(Sound&&);
		Sound& operator=(const Sound&) = delete;
		Sound& operator=(Sound&&);
		///@endcond

		/**
		 * @brief Convert the audio data into a form suitable for playback
		 *
		 * @throws BadRealizeStateException If the sound is already realized
		 * @throws BadInitStateException If the audio system is not initialized
		 */
		void Realize();

		/**
		 * @brief Destroy the realized representation of the asset
		 *
		 * @throws BadRealizeStateException If the sound is not realized
		 * @throws BadInitStateException If the audio system is not initialized
		 */
		void DropRealized();

		///@cond
		struct Impl;
		///@endcond

		~Sound();

	  private:
		Sound(std::vector<char>&& encodedAudio, const std::string& addr);
		friend class ResourceManager;

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;
	};
}