#include "libcacaoimage.hpp"
#include "libcacaocommon.hpp"
#include <cstddef>

#include "tga.h"

namespace libcacaoimage {
	//Inerface for TGA library to read/write with streams
	class StreamFileInterface : public tga::FileInterface {
	  public:
		StreamFileInterface(std::istream& stream)
		  : str(stream.rdbuf()), written(0), mode(Mode::In) {}
		StreamFileInterface(std::ostream& stream)
		  : str(stream.rdbuf()), written(0), mode(Mode::Out) {}

		StreamFileInterface(const StreamFileInterface&) = delete;
		StreamFileInterface(StreamFileInterface&&) = delete;
		StreamFileInterface& operator=(const StreamFileInterface&) = delete;
		StreamFileInterface& operator=(StreamFileInterface&&) = delete;

		bool ok() const override {
			return str.good();
		}

		size_t tell() override {
			return (mode == Mode::In ? str.tellg() : str.tellp());
		}

		void seek(size_t absPos) override {
			if(mode == Mode::In)
				str.seekg(absPos);
			else
				str.seekp(absPos);
		}

		uint8_t read8() override {
			if(mode == Mode::Out) return 0;
			uint8_t out;
			str.read(reinterpret_cast<char*>(&out), 1);
			return out;
		}

		void write8(uint8_t value) override {
			if(mode == Mode::In) return;
			str.write(reinterpret_cast<char*>(&value), 1);
			++written;
		}

		std::size_t getWritten() {
			return written;
		}

	  private:
		std::iostream str;
		std::size_t written;
		enum class Mode {
			In,
			Out
		} mode;
	};

	Image decode::DecodeTGA(std::istream& input) {
		//Create interface and decoder
		StreamFileInterface sfi(input);
		tga::Decoder dec(&sfi);

		//Read header to confirm TGA
		tga::Header head;
		CheckException(dec.readHeader(head), "Non-TGA data passed to DecodeTGA!");

		//Set image properties
		Image img;
		img.w = head.width;
		img.h = head.height;
		img.format = Image::Format::TGA;
		img.lossy = false;
		img.quality = 100;
		img.bitsPerChannel = 8;
		img.layout = (head.isRgb() ? (dec.hasAlpha() ? Image::Layout::RGBA : Image::Layout::RGB) : Image::Layout::Grayscale);

		//Decode image data
		tga::Image tga;
		tga.bytesPerPixel = head.bytesPerPixel();
		tga.rowstride = head.width * tga.bytesPerPixel;
		std::vector<unsigned char> buffer(tga.rowstride * head.height);
		tga.pixels = buffer.data();
		CheckException(dec.readImage(head, tga), "Failed to decode TGA image data!");

		//Post-process image data to fix up alpha
		dec.postProcessImage(head, tga);

		//If this image is opaque, we want to remove the alpha info
		//The TGA library always gives RGB data as RGBA, even if alpha is opaque
		//So if this IS opaque, we'd like to remove the extra info
		//This is why we don't write to the buffer immediately
		if(img.layout == Image::Layout::RGB) {
			//Setup output buffer
			const std::size_t pixels = head.width * head.height;
			img.data.resize(pixels * 3);

			//We're copying using raw pointers here because it's faster and also we know the buffer sizes are okay so we don't need bounds checking
			const unsigned char* src = buffer.data();
			unsigned char* dst = img.data.data();
			for(std::size_t i = 0; i < pixels; ++i) {
				//Copy R, G, and B values
				*dst++ = *src++;
				*dst++ = *src++;
				*dst++ = *src++;

				//Skip the A
				++src;
			}
		} else {
			img.data = std::move(buffer);
		}

		//Return result
		return img;
	}

	std::size_t encode::EncodeTGA(const Image& src, std::ostream& out) {
		//Input validation
		CheckException(src.w > 0 && src.h > 0, "Cannot encode an image with zeroed dimensions!");
		CheckException(src.bitsPerChannel == 8, "Invalid color depth for TGA encoding; only 8 bits per channel are supported.");
		CheckException(src.data.size() > 0, "Cannot encode an image with a zero-sized data buffer!");

		//Create interface and encoder
		StreamFileInterface sfi(out);
		tga::Encoder enc(&sfi);

		//Create header
		tga::Header head = {};
		head.width = src.w;
		head.height = src.h;
		head.idLength = 0;
		head.colormapType = 0;
		head.colormapLength = 0;
		head.colormap = {};
		head.imageDescriptor = 0x20;
		head.bitsPerPixel = static_cast<uint8_t>(src.layout) * 8;
		switch(src.layout) {
			case Image::Layout::Grayscale:
				head.imageType = tga::ImageType::RleGray;
				break;
			case Image::Layout::RGB:
				head.imageType = tga::ImageType::RleRgb;
				break;
			case Image::Layout::RGBA:
				head.imageType = tga::ImageType::RleRgb;
				head.imageDescriptor |= 0x8;
				break;
		}

		//Write the header
		enc.writeHeader(head);

		//Write the image data
		tga::Image img;
		img.bytesPerPixel = head.bitsPerPixel / 8;
		img.rowstride = img.bytesPerPixel * src.w;
		img.pixels = const_cast<uint8_t*>(src.data.data());
		enc.writeImage(head, img);

		//Write the footer
		enc.writeFooter();

		return sfi.getWritten();
	}
}