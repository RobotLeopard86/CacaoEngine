#include "libcacaoimage.hpp"
#include "libcacaocommon.hpp"

#include "tiff.h"
#include "tiffio.h"
#include "tiffio.hxx"

#include <iostream>
#include <memory>
#include <array>

namespace libcacaoimage {
	Image decode::DecodeTIFF(std::istream& input) {
		//Quick check to confirm TIFF
		std::array<unsigned char, 4> tiffSig;
		input.read(reinterpret_cast<char*>(tiffSig.data()), 4);
		CheckException(tiffSig[0] == 'I' && tiffSig[1] == 'I' && tiffSig[2] == '*' && tiffSig[3] == 0, "Non-TIFF data passed to DecodeTIFF!");
		input.seekg(0);

		//Open TIFF stream
		std::unique_ptr<TIFF, decltype(&TIFFClose)> tiff(TIFFStreamOpen("__memtiff", &input), TIFFClose);
		CheckException((bool)tiff, "Failed to load TIFF image data!");

		//Create variables for reading
		unsigned int width, height;
		uint16_t samplesPerPixel, bitsPerSample, photometric;

		//Get image properties
		CheckException(TIFFGetField(tiff.get(), TIFFTAG_IMAGEWIDTH, &width) == 1, "Failed to get image width!");
		CheckException(TIFFGetField(tiff.get(), TIFFTAG_IMAGELENGTH, &height) == 1, "Failed to get image height!");
		CheckException(TIFFGetFieldDefaulted(tiff.get(), TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel) == 1, "Failed to get image sample-per-pixel info!");
		CheckException(TIFFGetFieldDefaulted(tiff.get(), TIFFTAG_BITSPERSAMPLE, &bitsPerSample) == 1, "Failed to get image sample bitdepth info!");
		CheckException(TIFFGetField(tiff.get(), TIFFTAG_PHOTOMETRIC, &photometric) == 1, "Failed to get image photometrical mode!");

		//Store data in Image
		Image img;
		img.w = width;
		img.h = height;
		img.format = Image::Format::TIFF;
		img.lossy = false;
		img.quality = 100;
		CheckException(bitsPerSample == 8 || bitsPerSample == 16, "Unsupported sample bitdepth state; only 8 or 16-bit color is allowed!");
		img.bitsPerChannel = static_cast<uint8_t>(bitsPerSample);
		CheckException(samplesPerPixel <= 4 && samplesPerPixel >= 1 && samplesPerPixel != 2, "Unsupported sample-per-pixel state; only 8 or 16-bit color is allowed!");
		switch(samplesPerPixel) {
			case 1: img.layout = Image::Layout::Grayscale; break;
			case 3: img.layout = Image::Layout::RGB; break;
			case 4: img.layout = Image::Layout::RGBA; break;
			default: break;
		}

		//Calculate data for read
		const uint8_t bytesPerChnl = (bitsPerSample / 8);
		const std::size_t bytesPerPxl = samplesPerPixel * bytesPerChnl;
		const std::size_t pitch = bytesPerPxl * width;
		img.data.resize(pitch * height);

		//Tiled or scanlines?
		if(TIFFIsTiled(tiff.get())) {
			//Tiled (not fun)

			//Get tile dimensions
			unsigned int wtile = 0, htile = 0;
			CheckException(TIFFGetField(tiff.get(), TIFFTAG_TILEWIDTH, &width) == 1, "Failed to get image tile width!");
			CheckException(TIFFGetField(tiff.get(), TIFFTAG_TILELENGTH, &height) == 1, "Failed to get image tile height!");

			//Allocate tile storage buffer
			std::vector<unsigned char> tile(TIFFTileSize(tiff.get()));

			//Read tiles
			for(unsigned int y = 0; y < height; y += htile) {
				for(unsigned int x = 0; x < width; x += wtile) {
					//Read tile
					CheckException(TIFFReadEncodedTile(tiff.get(), TIFFComputeTile(tiff.get(), x, y, 0, 0), tile.data(), tile.size()) == 1, "Failed to read TIFF tile!");

					//Compute copy location
					const unsigned int wcpy = std::min(wtile, width - x);
					const unsigned int hcpy = std::min(htile, height - y);

					//Copy each tile row into the output buffer
					for(unsigned int r = 0; r < hcpy; ++r) {
						unsigned char* destination = img.data.data() + ((y + r) * pitch) + (x * bytesPerPxl);
						const unsigned char* source = tile.data() + (r * wtile * bytesPerPxl);
						std::memcpy(destination, source, wcpy * bytesPerPxl);
					}
				}
			}
		} else {
			//Scanlines (much more fun)
			for(unsigned int y = 0; y < height; ++y) {
				uint8_t* cpyDest = img.data.data() + (y * pitch);
				CheckException(TIFFReadScanline(tiff.get(), cpyDest, y, 0) == 1, "Failed to read TIFF scanline!");
			}
		}

		//Return result (TIFF will be automatically cleaned up because RAII and unique_ptr)
		return img;
	}

	void encode::EncodeTIFF(const Image& src, std::ostream& out) {
		//Input validation
		CheckException(src.w > 0 && src.h > 0, "Cannot encode an image with zeroed dimensions!");
		CheckException(src.bitsPerChannel == 8 || src.bitsPerChannel == 16, "Invalid color depth; only 8 and 16 are allowed.");
		CheckException(src.data.size() > 0, "Cannot encode an image with a zero-sized data buffer!");

		//Open TIFF stream
		std::unique_ptr<TIFF, decltype(&TIFFClose)> tiff(TIFFStreamOpen("__memtiff", &out), TIFFClose);
		CheckException((bool)tiff, "Failed to create TIFF encoder!");

		//Calculate values for encoding
		uint16_t samplesPerPixel = 0;
		uint16_t photometric;
		switch(src.layout) {
			case Image::Layout::Grayscale:
				samplesPerPixel = 1;
				photometric = PHOTOMETRIC_MINISBLACK;
				break;
			case Image::Layout::RGB:
				samplesPerPixel = 3;
				photometric = PHOTOMETRIC_RGB;
				break;
			case Image::Layout::RGBA:
				samplesPerPixel = 4;
				photometric = PHOTOMETRIC_RGB;

				//We also need to mark our extra channel as an alpha channel
				CheckException(TIFFSetField(tiff.get(), TIFFTAG_EXTRASAMPLES, 1, (uint16_t[]) {EXTRASAMPLE_ASSOCALPHA}) == 1, "Failed to mark image alpha channel!");
				break;
			default: break;
		}

		//Set image properties
		CheckException(TIFFSetField(tiff.get(), TIFFTAG_IMAGEWIDTH, src.w) == 1, "Failed to set image width property!");
		CheckException(TIFFSetField(tiff.get(), TIFFTAG_IMAGELENGTH, src.h) == 1, "Failed to set image height property!");
		CheckException(TIFFSetField(tiff.get(), TIFFTAG_BITSPERSAMPLE, src.bitsPerChannel) == 1, "Failed to set image bitdepth property!");
		CheckException(TIFFSetField(tiff.get(), TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel) == 1, "Failed to set image samples-per-pixel info!");
		CheckException(TIFFSetField(tiff.get(), TIFFTAG_PHOTOMETRIC, photometric) == 1, "Failed to set image photometrical mode!");
		CheckException(TIFFSetField(tiff.get(), TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) == 1, "Failed to set image planar configuration!");
		CheckException(TIFFSetField(tiff.get(), TIFFTAG_COMPRESSION, COMPRESSION_NONE) == 1, "Failed to set image compression mode!");
		CheckException(TIFFSetField(tiff.get(), TIFFTAG_ROWSPERSTRIP, 1) == 1, "Failed to set image strip size property!");

		//Calculate helper constants for encoding
		const std::size_t bytesPerChnl = (src.bitsPerChannel / 8);
		const std::size_t bytesPerPxl = bytesPerChnl * samplesPerPixel;
		const std::size_t pitch = bytesPerPxl * src.w;

		//Encode image data
		for(unsigned int y = 0; y < src.h; ++y) {
			unsigned char* row = const_cast<unsigned char*>(src.data.data() + (y * pitch));
			CheckException(TIFFWriteScanline(tiff.get(), row, y, 0) == 1, "Failed to encode TIFF scanline!");
		}

		//Flush stream to confirm write
		TIFFFlush(tiff.get());

		//Cleanup will, again, be handled by RAII and unique_ptr
	}
}