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
		uint8_t bitsPerChannel;			///<How many bits are in each image channel (8, 16, or 32)
		std::vector<unsigned char> data;///<Image buffer

		///@brief Supported formats
		enum class Format {
			PNG,
			JPEG,
			WebP,
			TGA,
			TIFF,
			KTX2
		} format;///<Original encoded format (set by decoder functions, useful for decoding and re-encoding)
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

		/**
		 * @brief Decode a KTX2 image
		 *
		 * If you know your image is KTX2, this avoids the overhead of format determination
		 *
		 * @param input An input stream to the encoded data
		 *
		 * @return The decoded image data
		 *
		 * @throws std::runtime_error If the data is not in KTX2 format or if decoding fails
		 */
		Image DecodeKTX(std::istream& input);
	}

	///@brief Image encoding functions
	namespace encode {
		/**
		 * @brief Re-encode a loaded image to its original format
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails
		 */
		void Reencode(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a PNG image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails
		 */
		void EncodePNG(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a JPEG image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails
		 */
		void EncodeJPEG(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a WebP image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails
		 */
		void EncodeWebP(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a TGA (Targa) image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails
		 */
		void EncodeTGA(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a TIFF image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails
		 */
		void EncodeTIFF(const Image& src, std::ostream& out);

		/**
		 * @brief Encode a KTX2 image
		 *
		 * @param src The image data to encode
		 * @param out The output stream to write the encoded data to
		 *
		 * @throws std::runtime_error If encoding fails
		 */
		void EncodeKTX(const Image& src, std::ostream& out);
	}
}