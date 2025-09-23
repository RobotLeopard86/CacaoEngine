#include "libcacaoimage.hpp"
#include "libcacaocommon.hpp"

#include <algorithm>
#include <cstdint>

namespace libcacaoimage {
	Image ChangeChannelLayout(const Image& src, Image::Layout layout) {
		CheckException(src.layout != layout, "Cannot change an image's channel layout to its current layout!");

		//Create copy image for result
		Image result = src;
		result.layout = layout;

		//Calculate some values we'll need later
		uint8_t oldChannels = static_cast<uint8_t>(src.layout);
		uint8_t newChannels = static_cast<uint8_t>(layout);
		unsigned int newBytesPerPixel = static_cast<unsigned int>(newChannels) * (result.bitsPerChannel / 8);
		std::size_t newPitch = static_cast<std::size_t>(newBytesPerPixel) * result.w;
		std::size_t pixelCount = result.w * result.h;

		//Reset result buffer
		result.data.resize(newPitch * result.h);
		result.data.shrink_to_fit();

		//Data conversion
		//We use raw pointers since the buffer sizes are known
		if(src.bitsPerChannel == 8) {
			const unsigned char* srcp = src.data.data();
			unsigned char* dstp = result.data.data();
			for(std::size_t i = 0; i < pixelCount; ++i) {
				//Special grayscale case
				if(layout == Image::Layout::Grayscale) {
					uint8_t r = (0.2126 * *srcp++);
					uint8_t g = (0.7152 * *srcp++);
					uint8_t b = (0.0722 * *srcp++);
					if(src.layout == Image::Layout::RGBA) ++srcp;
					uint16_t sum = r + g + b;
					*dstp++ = static_cast<uint8_t>(std::clamp<uint16_t>(sum, 0, 255));
					continue;
				}

				//This looks really funky but what it does is make sure that the correct number of bytes are copied
				//The outer loop makes sure that for grayscale -> RGB(A) conversion, we copy the same byte enough
				//The inner loop makes sure that the copy of each channel happens enough
				//The break is to make sure that we don't copy twice when going RGB -> RGBA
				uint8_t j = 0;
				do {
					for(uint8_t h = 0; h < std::min(oldChannels, newChannels); ++h) {
						*dstp++ = *srcp;
						srcp += std::clamp<uint8_t>(oldChannels - 1, 0, 1);
					}
					if(newChannels - oldChannels == 1) break;
				} while((j += oldChannels) < std::clamp<uint8_t>(newChannels, 1, 3));

				//Add opaque alpha if needed
				if(layout == Image::Layout::RGBA) {
					*dstp++ = UINT8_MAX;
				} else {
					++srcp;
				}
			}
		} else {
			//This is the same thing as above pretty much, just 16-bit
			const unsigned short* srcp = reinterpret_cast<const unsigned short*>(src.data.data());
			unsigned short* dstp = reinterpret_cast<unsigned short*>(result.data.data());
			for(std::size_t i = 0; i < pixelCount; ++i) {
				//Special grayscale case
				if(layout == Image::Layout::Grayscale) {
					uint16_t r = (0.2126 * *srcp++);
					uint16_t g = (0.7152 * *srcp++);
					uint16_t b = (0.0722 * *srcp++);
					if(src.layout == Image::Layout::RGBA) ++srcp;
					uint32_t sum = r + g + b;
					*dstp++ = static_cast<uint16_t>(std::clamp<uint32_t>(sum, 0, 255));
					continue;
				}

				//This looks really funky but what it does is make sure that the correct number of bytes are copied
				//The outer loop makes sure that for grayscale -> RGB(A) conversion, we copy the same byte enough
				//The inner loop makes sure that the copy of each channel happens enough
				//The break is to make sure that we don't copy twice when going RGB -> RGBA
				uint8_t j = 0;
				do {
					for(uint8_t h = 0; h < std::min(oldChannels, newChannels); ++h) {
						*dstp++ = *srcp;
						srcp += std::clamp<uint8_t>(oldChannels - 1, 0, 1);
					}
					if(newChannels - oldChannels == 1) break;
				} while((j += oldChannels) < std::clamp<uint8_t>(newChannels, 1, 3));

				//Add opaque alpha if needed
				if(layout == Image::Layout::RGBA) {
					*dstp++ = UINT16_MAX;
				} else {
					++srcp;
				}
			}
		}

		//Return result
		return result;
	}
}