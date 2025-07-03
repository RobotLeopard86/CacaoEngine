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
		///@cond
		Sound(const Sound&) = delete;
		Sound(Sound&&);
		Sound& operator=(const Sound&) = delete;
		Sound& operator=(Sound&&);
		///@endcond

		/**
		 * @brief Synchronously convert the audio data into a form suitable for playback
		 *
		 * @throws BadRealizeStateException If the sound is already realized
		 * @throws BadInitStateException If the audio system is not initialized
		 */
		void Realize();

		/**
		 * @brief Asynchronously convert the audio data into a form suitable for playback
		 *
		 * @return A future that will resolve when realization is complete or fails
		 *
		 * @throws BadRealizeStateException If the sound is already realized
		 * @throws BadInitStateException If the audio system is not initialized
		 */
		std::shared_future<void> RealizeAsync();

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

		std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		std::vector<char> encodedAudio;
	};
}