#include "libcacaoimage.hpp"
#include "libcacaocommon.hpp"

#include "webp/decode.h"
#include "webp/encode.h"

#include <array>

namespace libcacaoimage {
	Image decode::DecodeWebP(std::istream& input) {
		//Quick check to confirm WebP
		std::array<unsigned char, 12> wpSig;
		input.read(reinterpret_cast<char*>(wpSig.data()), 12);
		CheckException(wpSig[0] == 'R' && wpSig[1] == 'I' && wpSig[2] == 'F' && wpSig[3] == 'F' && wpSig[8] == 'W' && wpSig[9] == 'E' && wpSig[10] == 'B' && wpSig[11] == 'P', "Non-WebP data passed to DecodeWebP!");
		input.seekg(0);

		//Create decoding configuration object
		WebPDecoderConfig cfg;
		CheckException(WebPInitDecoderConfig(&cfg), "Failed to create libwebp decoder configuration!");

		//Read data into buffer
		std::vector<unsigned char> buffer = [&input]() {
			try {
				//Grab size
				input.clear();
				input.exceptions(std::ios::failbit | std::ios::badbit);
				input.seekg(0, std::ios::end);
				auto size = input.tellg();
				input.seekg(0, std::ios::beg);

				//Read data
				std::vector<unsigned char> contents(size);
				input.read(reinterpret_cast<char*>(contents.data()), size);

				return contents;
			} catch(std::ios_base::failure& ios_failure) {
				if(errno == 0) { throw ios_failure; }
				throw std::runtime_error("Failed to read WebP image data stream to buffer!");
			}
		}();

		//Get image features
		WebPBitstreamFeatures features;
		CheckException(WebPGetFeatures(buffer.data(), buffer.size(), &features) == VP8_STATUS_OK, "Failed to detect WebP image features!");
		CheckException(!features.has_animation, "Animated WebP images are not supported!");

		//Setup image object
		Image img;
		img.w = features.width;
		img.h = features.height;
		img.format = Image::Format::WebP;
		img.bitsPerChannel = 8;
		img.layout = features.has_alpha ? Image::Layout::RGBA : Image::Layout::RGB;
		img.quality = 80;
		img.lossy = true;

		//Setup decoder configuration
		cfg.options.no_fancy_upsampling = false;
		cfg.options.use_threads = true;
		cfg.output.colorspace = features.has_alpha ? MODE_RGBA : MODE_RGB;

		//Decode the image
		CheckException(WebPDecode(buffer.data(), buffer.size(), &cfg) == VP8_STATUS_OK, "Failed to decode WebP data!");

		//Extract data
		img.data.assign(cfg.output.u.RGBA.rgba, cfg.output.u.RGBA.rgba + cfg.output.u.RGBA.size);

		//Cleanup
		WebPFreeDecBuffer(&cfg.output);

		return img;
	}

	void encode::EncodeWebP(const Image& src, std::ostream& out) {
		//Input validation
		CheckException(src.w > 0 && src.h > 0, "Cannot encode an image with zeroed dimensions!");
		CheckException(src.bitsPerChannel == 8, "Invalid bit depth for WebP encoding; only 8 bits per channel are supported.");
		CheckException(src.data.size() > 0, "Cannot encode an image with a zero-sized data buffer!");
		CheckException(src.layout != Image::Layout::Grayscale, "Cannot encode a grayscale image to WebP format!");
		CheckException(src.quality >= 0 && src.quality <= 100, "Quality value must be between 0 and 100!");

		//Setup encoding mode
		uint8_t encMode = 0;
		encMode |= (src.layout == Image::Layout::RGBA ? 0xA : 0);
		encMode |= (src.lossy ? 0x70 : 0);

		//Encode image
		uint8_t* data = nullptr;
		std::size_t size = 0;
		switch(encMode) {
			case 0:
				//Lossless RGB
				size = WebPEncodeLosslessRGB(src.data.data(), src.w, src.h, src.w * 3, &data);
				break;
			case 0xA:
				//Lossless RGBA
				size = WebPEncodeLosslessRGBA(src.data.data(), src.w, src.h, src.w * 4, &data);
				break;
			case 0x70:
				//Lossy RGB
				size = WebPEncodeRGB(src.data.data(), src.w, src.h, src.w * 3, src.quality, &data);
				break;
			case 0x7A:
				//Lossy RGBA
				size = WebPEncodeRGBA(src.data.data(), src.w, src.h, src.w * 4, src.quality, &data);
				break;
		}
		CheckException(size > 0 && data != nullptr, "Failed to encode image to WebP!");

		//Write output
		out.write(reinterpret_cast<char*>(data), size);
	}
}