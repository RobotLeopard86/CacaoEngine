#pragma once

#include <vector>

namespace libcacaoformats {
	struct AudioBuffer {
		std::vector<short> data;
		uint64_t sampleCount;
		uint32_t sampleRate;
		uint8_t channelCount;
	};

	struct ImageBuffer {
		unsigned char* data;
		struct Size {
			uint32_t x, y;
		} size;
		uint8_t channelCount;
	};
}