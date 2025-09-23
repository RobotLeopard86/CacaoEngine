#pragma once

#include "Sound.hpp"
#include "World/Component.hpp"
#include "Utilities/MiscUtils.hpp"

namespace Cacao {
	/**
	 * @brief A component that plays audio
	 */
	class AudioPlayer final : public Component {
	  public:
		/**
		 * @brief Create a new audio player.
		 * @note Generally, you shouldn't call this constructor directly. Prefer creating this through Entity::MountComponent
		 *
		 * @throws Exception If the audio system is unitialized
		 */
		AudioPlayer();

		///@brief Delete the audio player
		~AudioPlayer();

		/**
		 * @brief Play the audio contained in sound
		 *
		 * @throws Exception If the audio system is uninitialized, a audio is already playing, or sound is a null handle
		 */
		void Play();

		/**
		 * @brief Pause or unpase audio playback
		 *
		 * @throws Exception If the audio system is uninitialized or no audio is playing
		 */
		void TogglePause();

		/**
		 * @brief Stop audio playback
		 *
		 * @throws Exception If the audio system is uninitialized or no audio is playing
		 */
		void Stop();

		/**
		 * @brief Check if a audio is playing
		 *
		 * @return Whether a audio is playing or not
		 *
		 * @throws Exception If the audio system is uninitialized
		 */
		bool IsPlaying();

		/**
		 * @brief Check if playback is paused
		 *
		 * @return Whether playback is paused or not
		 *
		 * @throws Exception If the audio system is uninitialized
		 */
		bool IsPaused();

		AssetHandle<Sound> sound;///<The sound that should be played

		/**
		 * @brief Set if the audio should loop
		 *
		 * @param val If the audio should loop
		 *
		 * @throws Exception If the audio system is uninitialized
		 */
		void SetLooping(bool val);

		/**
		 * @brief Set the gain value
		 *
		 * @param val The new gain value
		 *
		 * @throws Exception If the audio system is uninitialized
		 */
		void SetGain(float val);

		/**
		 * @brief Set the pitch multiplier
		 *
		 * @param val The new pitch multiplier
		 * 	- 0-1: Deeper
		 * 	- 1: Normal
		 * 	- > 1: Higher
		 *
		 * @throws Exception If the audio system is uninitialized
		 */
		void SetPitchMultiplier(float val);

		/**
		 * @brief Set how far through playback the player is
		 *
		 * @param timeInSeconds The new time in seconds
		 *
		 * @throws Exception If the audio system is uninitialized
		 */
		void SetPlaybackTime(float timeInSeconds);

		/**
		 * @brief Check if the audio is looping
		 *
		 * @return Whether the audio is looping or not
		 *
		 * @throws Exception If the audio system is uninitialized
		 */
		bool GetLooping();

		/**
		 * @brief Get the current gain value
		 *
		 * @return The current gain value
		 *
		 * @throws Exception If the audio system is uninitialized
		 */
		float GetGain();

		/**
		 * @brief Get the current pitch multiplier
		 *
		 * @return The current pitch multiplier
		 *
		 * @throws Exception If the audio system is uninitialized
		 */
		float GetPitchMultiplier();

		/**
		 * @brief Get the current playback time
		 *
		 * @return The current playback time in seconds
		 *
		 * @throws Exception If the audio system is uninitialized
		 */
		float GetPlaybackTime();

	  private:
		//OpenAL object
		ALuint source;

		SignalEventConsumer* sec;
		SignalEventConsumer* soundDelete;
		bool consumersSubscribed;
	};
}