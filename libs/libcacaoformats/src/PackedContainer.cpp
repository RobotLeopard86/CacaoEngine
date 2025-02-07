#include "libcacaoformats/libcacaoformats.hpp"

#include "bzlib.h"
#include "sha512.hh"

#include <cstdint>

#include "CheckException.hpp"

namespace libcacaoformats {
	PackedContainer::PackedContainer(FormatCode format, uint16_t ver, std::vector<unsigned char>&& data)
	  : format(format), version(ver), payload(data), hash(sw::sha512::calculate(payload.data(), payload.size() * sizeof(unsigned char))) {
		CheckException(payload.size() > 0, "Cannot make empty PackedContainer!");
	}

	PackedContainer PackedContainer::FromStream(std::istream& stream) {
		CheckException(stream.good(), "Data stream for packed container is invalid!");

		//Read 3 bytes to check Cacao Engine header
		std::string ceHeader(3, '\0');
		stream.read(ceHeader.data(), ceHeader.size());
		CheckException((static_cast<unsigned char>(ceHeader[0]) & 0xCA) == 0xCA && (static_cast<unsigned char>(ceHeader[1]) & 0xCA) == 0xCA && (static_cast<unsigned char>(ceHeader[2]) | 0x00) == 0x00, "Stream is not of a Cacao Engine packed object!");

		//Read next byte to check file type
		unsigned char type;
		stream.read(reinterpret_cast<char*>(&type), 1);
		FormatCode format;
		switch(type) {
			case 0xC4:
				format = FormatCode::Cubemap;
				break;
			case 0x1B:
				format = FormatCode::Shader;
				break;
			case 0x3E:
				format = FormatCode::Material;
				break;
			case 0xAA:
				format = FormatCode::AssetPack;
				break;
			case 0x7A:
				format = FormatCode::World;
				break;
			default:
				format = FormatCode::Material;//I have to set something here to avoid a compiler warning
				CheckException(false, "Stream is not a valid Cacao Engine format!");
				break;
		}

		//Get format version
		uint16_t version = 0;
		stream.read(reinterpret_cast<char*>(&version), 2);

		//Get hash
		std::string hash(128, '\0');
		stream.read(hash.data(), hash.size());

		//Get buffer size
		uint64_t uncompressedSize = 0;
		stream.read(reinterpret_cast<char*>(&uncompressedSize), 8);
		std::vector<unsigned char> uncompressed(uncompressedSize);

		{
			//Get compressed payload size
			std::streampos at = stream.tellg();
			stream.seekg(0, std::ios::end);
			std::size_t compressedSize = stream.tellg();
			stream.seekg(at);

			//Read compressed payload
			std::vector<unsigned char> compressed(compressedSize);
			stream.read(reinterpret_cast<char*>(compressed.data()), compressedSize);

			//Decompress payload
			unsigned int bzOutSizeSink = 0;

			int status = BZ2_bzBuffToBuffDecompress(reinterpret_cast<char*>(uncompressed.data()), &bzOutSizeSink, reinterpret_cast<char*>(compressed.data()), compressedSize, 0, 0);
			CheckException(status == BZ_OK, "Failed to decompress packed container payload!");
		}

		//Create output
		PackedContainer out(format, version, uncompressed);

		//Check hash
		CheckException(out.hash.compare(hash) == 0, "Packed container payload actual hash and provided hash differ!");

		//Return output
		return out;
	}

	void PackedContainer::ExportToStream(std::ostream& stream) {
		CheckException(stream.good(), "Output stream for packed container export is invalid!");

		//Write Cacao Engine magic number
		stream << 0xCACA00;

		//Write format code
		switch(format) {
			case FormatCode::AssetPack:
				stream << 0xAA;
				break;
			case FormatCode::Cubemap:
				stream << 0xC4;
				break;
			case FormatCode::Material:
				stream << 0x3E;
				break;
			case FormatCode::Shader:
				stream << 0x1B;
				break;
			case FormatCode::World:
				stream << 0x7A;
				break;
		}

		//Write hash
		stream << hash;

		//Write buffer size
		std::size_t bufSize = payload.size() * sizeof(unsigned char);
		stream << bufSize;

		//Compress payload
		unsigned int bzOutSizeSink = 0;
		std::vector<unsigned char> compressed(bufSize * 1.01 + 600);																														   //This size ratio comes from the bzip2 docs
		int status = BZ2_bzBuffToBuffCompress(reinterpret_cast<char*>(compressed.data()), &bzOutSizeSink, const_cast<char*>(reinterpret_cast<const char*>(payload.data())), bufSize, 9, 0, 30);//9 is the compression level, which is the default from the bzip2 command-line tool. 30 is the recommended work factor by the bzip2 docs.
		CheckException(status == BZ_OK, "Failed to compress payload for packed container export!");

		//Trim compressed data to free any memory not used
		compressed.shrink_to_fit();

		//Write compressed data
		for(unsigned char byte : compressed) {
			stream << byte;
		}

		//Flush output
		stream << std::flush;
	}
}