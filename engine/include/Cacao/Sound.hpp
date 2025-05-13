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
		 * @brief Synchronously convert the audio data into a form suitable for use
		 *
		 * @throws BadRealizeStateException If the sound is already realized
		 * @throws BadInitStateException If the audio system is not initialized
		 */
		void Realize();

		/**
		 * @brief Asynchronously convert the audio data into a form suitable for use
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
		std::unique_ptr<Impl> impl;
		///@endcond

	  private:
		std::vector<char> encodedAudio;

		Sound(std::vector<char>&& encoded);
		~Sound();
	};
}