#pragma once

#include <vector>

namespace libcacaoformats {
	struct AudioBuffer {
		::std::vector<short> data;
		uint64_t sampleCount;
		uint32_t sampleRate;
		uint8_t channelCount;
	};
}