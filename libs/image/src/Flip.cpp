#include "libcacaoimage.hpp"
#include "libcacaocommon.hpp"
#include <memory>

namespace libcacaoimage {
	Image Flip(const Image& src) {
		//Validate input
		CheckException(src.w > 0 && src.h > 0, "Cannot flip an image with zeroed dimensions!");
		CheckException(src.bitsPerChannel == 8 || src.bitsPerChannel == 16, "Invalid color depth; only 8 and 16 are allowed.");
		CheckException(src.data.size() > 0, "Cannot flip an image with a zero-sized data buffer!");

		//If the image is 1D, no flip can be done
		if(src.h == 1) return src;

		//Calculate row pitch
		const std::size_t rowPitch = static_cast<std::size_t>(src.w) * src.h * static_cast<uint8_t>(src.layout) * (src.bitsPerChannel / 8);

		//Allocate temporary row buffer
		std::unique_ptr<unsigned char[]> tempRow = std::make_unique_for_overwrite<unsigned char[]>(rowPitch);

		//Copy source image to output image to get properties
		Image out = src;

		//Flip the rows
		for(unsigned int y = 0; y < (src.h / 2); ++y) {
			//Calculate pointers for copy
			unsigned char* top = const_cast<unsigned char*>(src.data.data()) + static_cast<std::size_t>(y) * rowPitch;
			unsigned char* bottom = const_cast<unsigned char*>(src.data.data()) + static_cast<std::size_t>(src.h - y - 1) * rowPitch;

			//Copy top row into the temporary row buffer
			std::memcpy(tempRow.get(), top, rowPitch);

			//Copy bottom row into the top row
			std::memcpy(top, bottom, rowPitch);

			//Copy saved top row in the temporary buffer to the bottom row
			std::memcpy(bottom, tempRow.get(), rowPitch);
		}

		//Return result
		return out;
	}
}