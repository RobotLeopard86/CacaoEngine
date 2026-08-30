#pragma once

#include "DllHelper.hpp"
#include "Actor.hpp"
#include "Sound.hpp"

namespace Cacao {
	class CACAO_API ASTRA_REFLECT AudioPlayer final : public Component {
	  public:
		/**
		 * @brief Create a new audio player
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 */
		AudioPlayer();

		/**
		 * @brief Play or resume the audio
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 * @throws NonexistentValueException If there is no set sound to play
		 * @throws BadBakeStateException If the set sound is not baked
		 * @throws BadStateException If the audio is already playing
		 */
		ASTRA_IGNORE void Play();

		/**
		 * @brief Pause audio playback
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 * @throws BadStateException If no audio is playing
		 */
		ASTRA_IGNORE void Pause();

		/**
		 * @brief Stop audio playback
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 * @throws BadStateException If no audio is playing or paused
		 */
		ASTRA_IGNORE void Stop();

		/**
		 * @brief The current state of an audio player
		 */
		enum class ASTRA_IGNORE State {
			Playing,///<Audio is currently playing
			Paused, ///<Audio playback is paused and may be resumed
			Stopped ///<No audio is currently playing
		};

		/**
		 * @brief Check the current state of the audio player
		 *
		 * @return Current state
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 */
		ASTRA_IGNORE State GetState() const;

		/**
		 * @brief Set if the audio should loop
		 *
		 * @param val If the audio should loop
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 */
		ASTRA_IGNORE void SetLooping(bool val);

		/**
		 * @brief Set the gain value
		 *
		 * @param val The new gain value
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 * @throws BadValueException If the provided multiplier is negative
		 */
		ASTRA_IGNORE void SetGain(float val);

		/**
		 * @brief Set the pitch multiplier
		 *
		 * @param val The new pitch multiplier
		 * 	- 0-1: Deeper
		 * 	- 1: Normal
		 * 	- > 1: Higher
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 * @throws BadValueException If the provided multiplier is negative or zero
		 */
		ASTRA_IGNORE void SetPitchMultiplier(float val);

		/**
		 * @brief Set the playback position to the specified position in the sound
		 *
		 * @param position The new playback position in seconds
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 * @throws BadValueException If the provided position is negative or past the sound's end point
		 */
		ASTRA_IGNORE void SetPosition(float position);

		/**
		 * @brief Set the sound to be played (may be null)
		 *
		 * @param sound The new sound
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 */
		ASTRA_IGNORE void SetSound(std::shared_ptr<Sound> sound);

		/**
		 * @brief Check if the audio is looping
		 *
		 * @return Whether the audio is looping or not
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 */
		ASTRA_IGNORE bool GetLooping() const;

		/**
		 * @brief Get the current gain value
		 *
		 * @return The current gain value
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 */
		ASTRA_IGNORE float GetGain() const;

		/**
		 * @brief Get the current pitch multiplier
		 *
		 * @return The current pitch multiplier
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 */
		ASTRA_IGNORE float GetPitchMultiplier() const;

		/**
		 * @brief Get the current playback position
		 *
		 * @return The current playback position in seconds
		 *
		 * @throws BadInitStateException If the audio system is uninitialized
		 */
		ASTRA_IGNORE float GetPosition() const;

		/**
		 * @brief Get the current sound being played
		 *
		 * @return The current sound
		 */
		ASTRA_IGNORE std::shared_ptr<Sound> GetSound() const {
			return sound;
		}

		/**
		 * @brief Whether the audio player should start playing automatically when it's enabled
		 */
		bool autoplay;

		///@cond
		void OnEnable() override;
		void OnDisable() override;
		///@endcond

		ASTRASETUP(AudioPlayer)
		virtual ~AudioPlayer();

		///@cond
		struct Impl;
		///@endcond

	  private:
		ASTRA_IGNORE std::unique_ptr<Impl> impl;
		friend class ImplAccessor;

		ASTRA_IGNORE std::shared_ptr<Sound> sound;
	};
}