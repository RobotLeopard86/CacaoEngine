#pragma once

#include "libcacaoaudiodecode.hpp"

#include <vector>

namespace libcacaoaudiodecode {
	Result MP3Decode(std::vector<unsigned char> encoded);
	Result WAVDecode(std::vector<unsigned char> encoded);
	Result VorbisDecode(std::vector<unsigned char> encoded);
	Result OpusDecode(std::vector<unsigned char> encoded);
}