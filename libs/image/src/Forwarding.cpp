#include "libcacaoimage.hpp"

#include <stdexcept>
#include <array>
#include <cstring>

namespace libcacaoimage {
	Image decode::DecodeGeneric(std::istream& input) {
		//Read the first eighteen bytes (all detected types are within this range)
		std::array<unsigned char, 18> rbuf;
		input.read(reinterpret_cast<char*>(rbuf.data()), 4);
		input.seekg(0);

		//Check the bytes we got
		if(rbuf[0] == 0xFF && rbuf[1] == 0xD8 && rbuf[2] == 0xFF) {
			//JPEG
			return DecodeJPEG(input);
		} else if(rbuf[0] == 0x89 && rbuf[1] == 0x50 && rbuf[2] == 0x4E && rbuf[3] == 0x47 && rbuf[4] == 0x0D && rbuf[5] == 0x0A && rbuf[6] == 0x1A && rbuf[7] == 0x1A) {
			//PNG
			return DecodePNG(input);
		} else if(rbuf[0] == 'R' && rbuf[1] == 'I' && rbuf[2] == 'F' && rbuf[3] == 'F' && rbuf[8] == 'W' && rbuf[9] == 'E' && rbuf[10] == 'B' && rbuf[11] == 'P') {
			//WebP
			return DecodeWebP(input);
		} else if(rbuf[0] == 'I' && rbuf[1] == 'I' && rbuf[2] == '*' && rbuf[3] == 0) {
			//TIFF
			return DecodeTIFF(input);
		} else if(rbuf[0] == 0xAB && rbuf[1] == 'K' && rbuf[2] == 'T' && rbuf[3] == 'X' && rbuf[4] == ' ' && rbuf[5] == '2') {
			//KTX2
			return DecodeKTX(input);
		} else {
#pragma pack(push, 1)
			struct TGAHeader {
				uint8_t idLen;
				uint8_t colormapType;
				uint8_t imageType;
				uint16_t cmapFirstEntry;
				uint16_t cmapLen;
				uint8_t cmapEntrySz;
				uint16_t originX;
				uint16_t originY;
				uint16_t width;
				uint16_t height;
				uint8_t pixelDepth;
				uint8_t imageDescriptor;
			};
#pragma pack(pop)

			//Try parsing TGA header
			TGAHeader tga = {};
			std::memcpy(&tga, rbuf.data(), sizeof(TGAHeader));

			//Do checks to validate header (if this isn't TGA these should fail)
			if(tga.colormapType > 1) goto no;
			if(tga.imageType == 0) goto no;
			if(tga.width < 1 || tga.height < 1) goto no;
			if(tga.pixelDepth != 8 && tga.pixelDepth != 15 && tga.pixelDepth != 16 && tga.pixelDepth != 24 && tga.pixelDepth != 32) goto no;
			if(tga.colormapType == 1) {
				if(tga.imageType != 1 && tga.imageType != 9) goto no;
				if(tga.cmapEntrySz != 8 && tga.cmapEntrySz != 15 && tga.cmapEntrySz != 16 && tga.cmapEntrySz != 24 && tga.cmapEntrySz != 32) goto no;
			} else {
				if(tga.imageType != 2 && tga.imageType != 3 && tga.imageType != 10 && tga.imageType != 11) goto no;
			}

			//If we made it this far it should be TGA
			return DecodeTGA(input);
		}

	no:
		throw std::runtime_error("Unknown file type!");
	}

	void encode::Reencode(const Image& src, std::ostream& out) {
		switch(src.format) {
			case Image::Format::PNG: return EncodePNG(src, out);
			case Image::Format::JPEG: return EncodePNG(src, out);
			case Image::Format::WebP: return EncodePNG(src, out);
			case Image::Format::TGA: return EncodePNG(src, out);
			case Image::Format::TIFF: return EncodePNG(src, out);
			case Image::Format::KTX2: return EncodePNG(src, out);
			default: throw std::runtime_error("Invalid image format!");
		}
	}
}