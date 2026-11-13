#pragma once

#include "libcacaoaudiodecode.hpp"

#include <vector>

namespace libcacaoaudiodecode {
	Result MP3Decode(const std::vector<unsigned char>& encoded);
	Result WAVDecode(const std::vector<unsigned char>& encoded);
	Result VorbisDecode(const std::vector<unsigned char>& encoded);
	Result OpusDecode(const std::vector<unsigned char>& encoded);
}