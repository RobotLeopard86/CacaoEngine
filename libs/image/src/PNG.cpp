#include "libcacaoimage.hpp"
#include "libcacaocommon.hpp"

#include "png.h"

#include <iostream>

namespace libcacaoimage {
	Image decode::DecodePNG(std::istream& input) {
		//Quick check to confirm PNG
		std::array<unsigned char, 8> pngSig;
		input.read(reinterpret_cast<char*>(pngSig.data()), 8);
		CheckException(png_sig_cmp(pngSig.data(), 0, 8) == 0, "Non-PNG data passed to DecodePNG!");
		input.seekg(0);

		//Initialize libpng
		png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		CheckException(png, "Failed to initialize libpng!");
		png_infop info = png_create_info_struct(png);
		CheckException(info, "Failed to set up libpng information!", [&png]() { png_destroy_read_struct(&png, nullptr, nullptr); });

		//Configure libpng longjmp
		int sj = setjmp(png_jmpbuf(png));
		CheckException(sj == 0, "libpng read error!", [&png, &info]() { png_destroy_read_struct(&png, &info, nullptr); });

		//Set up IO read callback
		png_set_read_fn(png, &input, [](png_structp png, png_bytep bytesOut, png_size_t readBytes) {
			//Obtain the stream
			std::istream* stream = static_cast<std::istream*>(png_get_io_ptr(png));
			if(!stream) png_error(png, "Failed to retrieve input data stream!");

			//Get the data
			if(!stream->read(reinterpret_cast<char*>(bytesOut), readBytes)) png_error(png, "Failed to read data from input stream!");
		});

		//Force sRGB
		png_set_gamma(png, 2.2, 0.45455);

		//Load PNG info
		png_read_info(png, info);

		//Get image characteristics
		Image img;
		img.format = Image::Format::PNG;
		int bitdepth = -1, colortype = -1;
		png_get_IHDR(png, info, &img.w, &img.h, &bitdepth, &colortype, nullptr, nullptr, nullptr);

		//De-paletteization
		if(colortype == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);

		//Bit expansion
		if(colortype == PNG_COLOR_TYPE_GRAY && bitdepth < 8) png_set_expand_gray_1_2_4_to_8(png);
		png_set_packing(png);

		//No gray+alpha
		CheckException(colortype != PNG_COLOR_TYPE_GA, "Grayscale images with alpha channels are unsupported!", [&png, &info]() { png_destroy_read_struct(&png, &info, nullptr); });

		//Turn tRNS info into alpha if needed
		if(png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);

		//Fill alpha if there isn't any
		png_set_add_alpha(png, bitdepth == 8 ? 0xFF : 0xFFFF, PNG_FILLER_AFTER);

		//Swap byte order for endianness if necessary
		if(bitdepth == 16) {
			if constexpr(std::endian::native == std::endian::big) png_set_swap(png);
		}

		//Process transformation data
		png_read_update_info(png, info);
		png_get_IHDR(png, info, &img.w, &img.h, &bitdepth, &colortype, nullptr, nullptr, nullptr);
		img.bitsPerChannel = bitdepth == 16 ? 16 : 8;

		//Get channel layout
		uint8_t channels = png_get_channels(png, info);
		CheckException(channels <= 4 && channels > 0 && channels != 2, "Invalid channel layout detected!", [&png, &info]() { png_destroy_read_struct(&png, &info, nullptr); });
		switch(channels) {
			case 1: img.layout = Image::Layout::Grayscale; break;
			case 3: img.layout = Image::Layout::RGB; break;
			case 4: img.layout = Image::Layout::RGBA; break;
			default: break;
		}

		//Prepare data buffer
		std::size_t bytesPerRow = png_get_rowbytes(png, info);
		img.data.resize(bytesPerRow * img.h);
		std::vector<png_bytep> rowPointers(img.h);
		for(std::size_t y = 0; y < img.h; y++) {
			rowPointers[y] = img.data.data() + (y * bytesPerRow);
		}

		//Read the decoded pixels
		png_read_image(png, rowPointers.data());
		png_read_end(png, info);

		//Cleanup libpng
		png_destroy_read_struct(&png, &info, nullptr);

		//Return result
		return img;
	}

	void encode::EncodePNG(const Image& src, std::ostream& out) {
		//Input validation
		CheckException(src.w > 0 && src.h > 0, "Cannot encode an image with zeroed dimensions!");
		CheckException(src.bitsPerChannel == 8 || src.bitsPerChannel == 16, "Invalid bit depth for PNG encoding; 8 or 16 is required.");
		CheckException(src.data.size() > 0, "Cannot encode an image with a zero-sized data buffer!");

		//Initialize libpng
		png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		CheckException(png, "Failed to initialize libpng!");
		png_infop info = png_create_info_struct(png);
		CheckException(info, "Failed to set up libpng information!", [&png]() { png_destroy_write_struct(&png, nullptr); });

		//Configure libpng longjmp
		int sj = setjmp(png_jmpbuf(png));
		CheckException(sj == 0, "libpng write error!", [&png, &info]() { png_destroy_write_struct(&png, &info); });

		// clang-format off

		//Set up IO write callback
		png_set_write_fn(png, &out, [](png_structp png, png_bytep bytesIn, png_size_t writeBytes) {
			//Obtain the stream
			std::ostream* stream = static_cast<std::ostream*>(png_get_io_ptr(png));
			if(!stream) png_error(png, "Failed to retrieve output data stream!");

			//Write the data
			if(!stream->write(reinterpret_cast<char*>(bytesIn), writeBytes)) png_error(png, "Failed to write data to output stream!"); 
		}, [](png_structp png) {
			//Obtain the stream
			std::ostream* stream = static_cast<std::ostream*>(png_get_io_ptr(png));
			if(!stream) png_error(png, "Failed to retrieve output data stream!");

			//Flush it
			stream->flush(); 
		});
		// clang-format on

		//Create image write struct
		int colortype = -1, channelcount = -1;
		switch(src.layout) {
			case Image::Layout::Grayscale:
				colortype = PNG_COLOR_TYPE_GRAY;
				channelcount = 1;
				break;
			case Image::Layout::RGB:
				colortype = PNG_COLOR_TYPE_RGB;
				channelcount = 3;
				break;
			case Image::Layout::RGBA:
				colortype = PNG_COLOR_TYPE_RGBA;
				channelcount = 4;
				break;
			default: break;
		}
		png_set_IHDR(png, info, src.w, src.h, src.bitsPerChannel, colortype, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		//Declare sRGB and gamma
		png_set_sRGB_gAMA_and_cHRM(png, info, PNG_sRGB_INTENT_PERCEPTUAL);

		//Hand over image data
		std::vector<png_bytep> rowPointers(src.h);
		for(std::size_t y = 0; y < src.h; y++) {
			rowPointers[y] = const_cast<unsigned char*>(src.data.data()) + (y * src.w * channelcount * (src.bitsPerChannel / 8));
		}
		png_set_rows(png, info, rowPointers.data());

		//Do we need to swap byte order for endianness?
		bool swapBytes = std::endian::native == std::endian::big && src.bitsPerChannel == 16;

		//Write the image
		png_write_png(png, info, (swapBytes ? PNG_TRANSFORM_SWAP_ENDIAN : PNG_TRANSFORM_IDENTITY), nullptr);

		//Cleanup libpng
		png_destroy_write_struct(&png, &info);
	}
}