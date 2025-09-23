#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace libcacaoaudiodecode {
	///@brief Decoded audio data and properties necessary to use it
	struct Result {
		std::vector<short> data;///<Audio data
		uint64_t sampleCount;	///<Number of audio samples
		uint32_t sampleRate;	///<Rate of samples per second
		uint8_t channelCount;	///<Audio channel count
	};

	/**
	 * @brief Convenience function for decoding audio data
	 *
	 * @details Supports MP3, WAV, Ogg Vorbis, and Ogg Opus
	 *
	 * @param encoded The encoded audio data to decode, provided via stream
	 *
	 * @return The decoded audio buffer and metadata
	 *
	 * @throws std::runtime_error If the provided data is not of the correct size, is of an unsupported format, or the audio decoding fails
	 */
	Result DecodeAudio(std::istream& encoded);
}