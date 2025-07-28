#pragma once

#include <vector>
#include <istream>
#include <ostream>
#include <cstdint>

namespace libcacaoimage {
	///@brief Decoded image representation
	struct Image {
		unsigned int w;///<Width of image in pixels
		unsigned int h;///<Height of image in pixels

		///@brief Possible channel layout in of data buffer
		enum class Layout {
			Grayscale,					///<One channel
			RGB,						///<Three channels
			RGBA						///<Four channels
		} layout;						///<Layout of data buffer
		uint8_t bitsPerChannel;			///<How many bits are in each image channel (only 8 or 16)
		std::vector<unsigned char> data;///<Image buffer

		///@brief Supported formats
		enum class Format {
			PNG, ///<PNG
			JPEG,///<JPEG
			WebP,///<WebP
			TGA, ///<TGA (Targa)
			TIFF,///<TIFF
		} format;///<Original encoded format (set by decoder functions, useful for decoding and re-encoding)

		int quality;///<0-100, image quality for encoding in supported formats
		bool lossy; ///<Whether to use lossy compression when encoding in supported formats
	};

	///@brief Image decoding functions
	namespace decode {
		/**
		 * @brief Decode an arbitrary image of an unknown format
		 *
		 * @param input An input stream to the encoded data
		 *
		 * @return The decoded image data
		 *
		 * @throws std::runtime_error If the format cannot be determined or if decoding fails
		 */
		Image DecodeGeneric(std::istream& input);

		/**
		 * @brief Decode a PNG image
		 *
		 * If you know your image is PNG, this avoids the overhead of format determination
		 *
		 * @param input An input stream to the encoded data
		 *
		 * @return The decoded image data
		 *
		 * @throws std::runtime_error If the data is not in PNG format or if decoding fails
		 */
		Image DecodePNG(std::istream& input);

		/**
		 * @brief Decode a JPEG image
		 *
		 * If you know your image is JPEG, this avoids the overhead of format determination
		 *
		 * @param input An input stream to the encoded data
		 *
		 * @return The decoded image data
		 *
		 * @throws std::runtime_error If the data is not in JPEG format or if decoding fails
		 */
		Image DecodeJPEG(std::istream& input);

		/**
		 * @brief Decode a WebP image
		 *
		 * If you know your image is WebP, this avoids the overhead of format determination
		 *
		 * @param input An input stream to the encoded data
		 *
		 * @return The decoded image data
		 *
		 * @throws std::runtime_error If the data is not in WebP format or if decoding fails
		 */
		Image DecodeWebP(std::istream& input);

		/**
		 * @brief Decode a TGA (Targa) image
		 *
		 * If you know your image is TGA, this avoids the overhead of format determination
		 *
		 * @param input An input stream to the encoded data
		 *
		 * @return The decoded image data
		 *
		 * @throws std::runtime_error If the data is not in TGA format or if decoding fails
		 */
		Image DecodeTGA(std::istream& input);

		/**
		 * @brief Decode a TIFF image
		 *
		 * If you know your image is TIFF, this avoids the overhead of format determination
		 *
		 * @param input An input stream to the encoded data
		 *
		 * @return The decoded image data
		 *
		 * @throws std::runtime_error If the data is not in TIFF format or if decoding fails
		 */
		Image DecodeTIFF(std::istream& input);
	}

	///@brief Image encoding functions
	namespace encode {
		/**
		 * @brief Re-encode a loaded image to its original format
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails or settings or invalid
		 */
		void Reencode(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a PNG image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails or settings or invalid
		 */
		void EncodePNG(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a JPEG image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails or settings or invalid
		 */
		void EncodeJPEG(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a WebP image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails or settings or invalid
		 */
		void EncodeWebP(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a TGA (Targa) image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails or settings or invalid
		 */
		void EncodeTGA(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a TIFF image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails or settings or invalid
		 */
		void EncodeTIFF(const Image& src, std::ostream& out);
	}

	/**
	 * @brief Convert an Image with a 16-bit color depth to one with an 8-bit color depth
	 *
	 * @param src The source 16-bit image
	 *
	 * @return A new image with 8-bit color depth
	 *
	 * @throws std::runtime_error If the source image is not 16-bit
	 */
	Image Convert16To8BitColor(const Image& src);
}