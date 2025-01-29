#include "libcacaoformats/libcacaoformats.hpp"

#include "stb_image.h"
#include "stb_image_write.h"

#include "CheckException.hpp"

#include <cstring>

namespace libcacaoformats {
	ImageBuffer DecodeImage(std::istream encoded) {
		CheckException(encoded.good(), "Encoded data stream for image is invalid!");

		//Read input stream into buffer
		std::vector<unsigned char> encodedBuf = [&encoded]() {
			try {
				//Grab size
				encoded.exceptions(std::ios::failbit | std::ios::badbit);
				encoded.seekg(0, std::ios::end);
				auto size = encoded.tellg();
				encoded.seekg(0, std::ios::beg);

				//Read data
				std::vector<unsigned char> contents(size);
				encoded.read(reinterpret_cast<char*>(contents.data()), size);

				return contents;
			} catch(std::ios_base::failure& ios_failure) {
				if(errno == 0) { throw ios_failure; }
				CheckException(false, "Failed to read encoded image data stream!");
			}
		}();

		//Create output buffer
		ImageBuffer out;

		//Load data
		unsigned char* data = stbi_load_from_memory(encodedBuf.data(), encodedBuf.size() * sizeof(unsigned char), reinterpret_cast<int*>(&out.size.x), reinterpret_cast<int*>(&out.size.y), reinterpret_cast<int*>(&out.channelCount), 0);
		if(!data) {
			//We still have to free the junk
			stbi_image_free(data);

			CheckException(false, "Failed to decode image buffer!");
		}

		//Copy output data
		std::size_t decodedSize = out.size.x * out.size.y * out.channelCount;
		out.data = std::vector<unsigned char>(decodedSize);
		std::memcpy(out.data.data(), data, decodedSize);

		//Free initial buffer
		stbi_image_free(data);

		return out;
	}

	void EncodeImage(const ImageBuffer& img, std::ostream out) {
		CheckException(img.channelCount > 0 && img.size.x > 0 && img.size.y > 0, "Image buffer to encode has invalid dimensions or channel count!");
		CheckException(!img.data.empty(), "Image buffer to encode has empty data buffer!");

		//Write the data out
		stbi_write_png_to_func([](void* ctx, void* data, int) {
			//For anyone who thinks this is funky: this is just obtaining the out stream via the context pointer and writing the data into it in one line
			*(reinterpret_cast<std::ostream*>(ctx)) << data;
		},
			&out, img.size.x, img.size.y, img.channelCount, img.data.data(), 0);
	}
}