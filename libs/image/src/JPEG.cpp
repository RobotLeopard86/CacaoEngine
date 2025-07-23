#include "libcacaoimage.hpp"
#include "libcacaocommon.hpp"

#include "turbojpeg.h"

namespace libcacaoimage {
	Image decode::DecodeJPEG(std::istream& input) {
		//Quick check to confirm JPEG
		std::array<unsigned char, 3> jpgSig;
		input.read(reinterpret_cast<char*>(jpgSig.data()), 8);
		CheckException(jpgSig[0] == 0xFF && jpgSig[1] == 0xD8 && jpgSig[2] == 0xFF, "Non-JPEG data passed to DecodeJPEG!");
		input.seekg(0);

		//Initialize TurboJPEG
		tjhandle tj = tj3Init(TJINIT_DECOMPRESS);
		CheckException(tj, "Failed to initialize TurboJPEG!");

		//Read out buffer
		std::vector<unsigned char> buffer = [&input, &tj]() {
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
				tj3Destroy(tj);
				if(errno == 0) { throw ios_failure; }
				throw std::runtime_error("Failed to read JPEG image data stream to buffer!");
			}
		}();

		//Parse JPEG header
		CheckException(tj3DecompressHeader(tj, buffer.data(), buffer.size()) == 0, "Failed to parse JPEG header!", [&tj]() { tj3Destroy(tj); });

		//Get image information
		Image img;
		img.w = tj3Get(tj, TJPARAM_JPEGWIDTH);
		img.h = tj3Get(tj, TJPARAM_JPEGHEIGHT);
		img.format = Image::Format::JPEG;
		int bitdepth = tj3Get(tj, TJPARAM_PRECISION);
		int colorspace = tj3Get(tj, TJPARAM_COLORSPACE);

		//Can we handle this JPEG?
		CheckException(bitdepth >= 8 && bitdepth <= 16, "Invalid bits per channel in JPEG!", [&tj]() { tj3Destroy(tj); });
		CheckException(colorspace != TJCS_CMYK, "CMYK JPEGs are not supported!", [&tj]() { tj3Destroy(tj); });
		img.layout = (colorspace == TJCS_GRAY ? Image::Layout::Grayscale : Image::Layout::RGB);

		//Bits-per-channel normalization
		img.bitsPerChannel = (bitdepth >= 9) ? 16 : 8;

		//Create output buffer
		int pxSize = (img.layout == Image::Layout::RGB ? 3 : 1) * (img.bitsPerChannel / 8);
		img.data = std::vector<unsigned char>(img.w * img.h * pxSize);

		//Decode JPEG
		if(img.bitsPerChannel == 8) {
			CheckException(tj3Decompress8(tj, buffer.data(), buffer.size(), img.data.data(), img.w * pxSize, (img.layout == Image::Layout::RGB ? TJPF_RGB : TJPF_GRAY)) == 0,
				"Failed to decode JPEG!", [&tj]() { tj3Destroy(tj); });
		} else {
			if(bitdepth > 12) {
				//Create temporary buffer for 12-bit data
				std::vector<int16_t> twelveBit(img.w * img.h * (pxSize / 2));
				CheckException(tj3Decompress12(tj, buffer.data(), buffer.size(), twelveBit.data(), img.w * pxSize, (img.layout == Image::Layout::RGB ? TJPF_RGB : TJPF_GRAY)) == 0,
					"Failed to decode JPEG!", [&tj]() { tj3Destroy(tj); });

				//Map to 16-bit colorspace
				uint16_t* imgDataSixteen = reinterpret_cast<uint16_t*>(img.data.data());
				for(std::size_t i = 0; i < twelveBit.size(); ++i) {
					imgDataSixteen[i] = uint16_t(twelveBit[i]) << 4;
				}
			} else {
				CheckException(tj3Decompress16(tj, buffer.data(), buffer.size(), reinterpret_cast<uint16_t*>(img.data.data()), img.w * pxSize, (img.layout == Image::Layout::RGB ? TJPF_RGB : TJPF_GRAY)) == 0,
					"Failed to decode JPEG!", [&tj]() { tj3Destroy(tj); });
			}
		}

		//Cleanup
		tj3Destroy(tj);

		return img;
	}

	void encode::EncodeJPEG(const Image& src, std::ostream& out) {
	}
}