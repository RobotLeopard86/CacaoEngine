#pragma once

#include "libcacaoformats/libcacaoformats.hpp"

#include <vector>

namespace libcacaoformats {
	AudioBuffer MP3Decode(std::vector<unsigned char> encoded);
	AudioBuffer WAVDecode(std::vector<unsigned char> encoded);
	AudioBuffer VorbisDecode(std::vector<unsigned char> encoded);
	AudioBuffer OpusDecode(std::vector<unsigned char> encoded);
}