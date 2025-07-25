#include "libcacaoimage.hpp"
#include "libcacaocommon.hpp"

#include "turbojpeg.h"

#include <array>

namespace libcacaoimage {
	Image decode::DecodeJPEG(std::istream& input) {
		//Quick check to confirm JPEG
		std::array<unsigned char, 3> jpgSig;
		input.read(reinterpret_cast<char*>(jpgSig.data()), 3);
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
		img.quality = 80;
		img.lossy = img.bitsPerChannel == 8;

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
		//Input validation
		CheckException(src.w > 0 && src.h > 0, "Cannot encode an image with zeroed dimensions!");
		CheckException(src.bitsPerChannel == 8 || src.bitsPerChannel == 16, "Invalid bit depth; only 8 and 16 are allowed.");
		CheckException(src.data.size() > 0, "Cannot encode an image with a zero-sized data buffer!");
		CheckException(src.layout != Image::Layout::RGBA, "Cannot encode an image with an alpha channel to JPEG format!");
		if(src.lossy) {
			CheckException(src.bitsPerChannel == 8, "Lossy compression is not supported for JPEG images with 16-bit color!");
			CheckException(src.quality >= 0 && src.quality <= 100, "Quality value must be between 0 and 100!");
		} else {
			CheckException(src.bitsPerChannel == 16, "Lossless compression is not supported for JPEG images with 8-bit color!");
		}

		//Initialize TurboJPEG
		tjhandle tj = tj3Init(TJINIT_COMPRESS);
		CheckException(tj, "Failed to initialize TurboJPEG!");

		//Make output buffer (maximum size needed, will almost certainly be trimmed down)
		std::vector<unsigned char> outBuf(tj3JPEGBufSize(src.w, src.h, TJSAMP_444));

		//Configure settings
		CheckException(tj3Set(tj, TJPARAM_NOREALLOC, true) == 0, "Failed to disable TurboJPEG auto-reallocation!", [&tj]() { tj3Destroy(tj); });
		CheckException(tj3Set(tj, TJPARAM_PRECISION, src.bitsPerChannel) == 0, "Failed to set TurboJPEG precision!", [&tj]() { tj3Destroy(tj); });
		if(src.lossy) CheckException(tj3Set(tj, TJPARAM_QUALITY, src.quality) == 0, "Failed to set TurboJPEG quality!", [&tj]() { tj3Destroy(tj); });
		CheckException(tj3Set(tj, TJPARAM_SUBSAMP, TJSAMP_444) == 0, "Failed to set TurboJPEG subsampling settings!", [&tj]() { tj3Destroy(tj); });

		//Encode the data
		int pixelFormat = (src.layout == Image::Layout::RGB ? TJPF_RGB : TJPF_GRAY);
		int pitch = src.w * (src.layout == Image::Layout::RGB ? 3 : 1);
		std::size_t outSize = outBuf.size();
		if(src.bitsPerChannel == 8) {
			unsigned char* outPtr = outBuf.data();
			CheckException(tj3Compress8(tj, src.data.data(), src.w, pitch, src.h, pixelFormat, &outPtr, &outSize) == 0, "Failed to encode JPEG data!", [&tj]() { tj3Destroy(tj); });
		} else {
			unsigned char* outPtr = outBuf.data();
			CheckException(tj3Compress16(tj, reinterpret_cast<const uint16_t*>(src.data.data()), src.w, pitch, src.h, pixelFormat, &outPtr, &outSize) == 0,
				"Failed to encode JPEG data!", [&tj]() { tj3Destroy(tj); });
		}

		//Trim the buffer to remove unnecessary end padding
		outBuf.resize(outSize);

		//Write output to stream
		out.write(reinterpret_cast<char*>(outBuf.data()), outBuf.size());

		//Cleanup
		tj3Destroy(tj);
	}
}