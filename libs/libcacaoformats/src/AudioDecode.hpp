#pragma once

#include "libcacaoformats/libcacaoformats.hpp"

#include <string>

namespace libcacaoformats {
    AudioBuffer MP3Decode(std::string filePath);
    AudioBuffer WAVDecode(std::string filePath);
    AudioBuffer VorbisDecode(std::string filePath);
    AudioBuffer OpusDecode(std::string filePath);
    AudioBuffer AnyDecode(std::string filePath)
}