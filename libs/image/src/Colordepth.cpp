#include "libcacaoimage.hpp"
#include "libcacaocommon.hpp"

#include "colordepth_lut.hpp"

namespace libcacaoimage {
	Image Convert16To8BitColor(const Image& src) {
		CheckException(src.bitsPerChannel == 16, "Source image for 16 to 8-bit color conversion does not have a 16-bit color depth!");

		//Setup image
		Image out = {};
		out.w = src.w;
		out.h = src.h;
		out.format = src.format;
		out.layout = src.layout;
		out.bitsPerChannel = 8;
		out.lossy = src.lossy;
		out.quality = src.quality;

		//Prepare for conversion
		out.data.resize(src.data.size() / 2);
		const uint8_t channels = static_cast<uint8_t>(src.layout);
		const std::size_t pixels = src.data.size() / channels / 2;

		//Convert data
		for(std::size_t px = 0; px < pixels; ++px) {
			for(uint8_t channel = 0; channel < channels; ++channel) {
				//Calculate index into input buffer
				const std::size_t index = (px * channels + channel) * 2;

				//Get the 16-bit value
				uint16_t value = static_cast<uint16_t>(src.data[index]) | (static_cast<uint16_t>(src.data[index + 1]) << 8);

				//Convert to 8-bit and store
				out.data[px * channels + channel] = lut[value];
			}
		}

		//Return result
		return out;
	}
}