#include "libcacaoformats.hpp"

#include "bzlib.h"
#include "libcacaocommon.hpp"

#include <cstdint>

namespace libcacaoformats {
	std::vector<unsigned char> Unsign(const std::vector<char>& vec) {
		std::vector<unsigned char> out(vec.size());
		std::memcpy(out.data(), vec.data(), vec.size() * sizeof(unsigned char));
		return out;
	}

	std::vector<char> Sign(const std::vector<unsigned char>& vec) {
		std::vector<char> out(vec.size());
		std::memcpy(out.data(), vec.data(), vec.size() * sizeof(char));
		return out;
	}

	PackedContainer::PackedContainer(PackedFormat format, uint16_t ver, std::vector<unsigned char>&& data)
	  : format(format), version(ver), payload(data) {
		CheckException(payload.size() > 0, "Cannot make empty PackedContainer!");
	}

	PackedContainer::PackedContainer(PackedFormat format, uint16_t ver, std::vector<char>&& data)
	  : format(format), version(ver), payload(Unsign(data)) {
		CheckException(payload.size() > 0, "Cannot make empty PackedContainer!");
	}

	PackedContainer PackedContainer::FromAsset(const PackedAsset& asset) {
		CheckException(asset.kind == PackedAsset::Kind::Cubemap || asset.kind == PackedAsset::Kind::Material || asset.kind == PackedAsset::Kind::Shader, "Cannot make PackedContainer from asset that is not a cubemap, material, or shader!");

		//Create buffer and stream
		std::vector<char> buffer = Sign(asset.buffer);
		ibytestream stream(buffer);
		return FromStream(stream);
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
		PackedFormat format;
		switch(type) {
			case 0xC4:
				format = PackedFormat::Cubemap;
				break;
			case 0x1B:
				format = PackedFormat::Shader;
				break;
			case 0x3E:
				format = PackedFormat::Material;
				break;
			case 0xAA:
				format = PackedFormat::AssetPack;
				break;
			case 0x7A:
				format = PackedFormat::World;
				break;
			default:
				format = PackedFormat::Material;//I have to set something here to avoid a compiler warning
				CheckException(false, "Stream is not a valid Cacao Engine format!");
				break;
		}

		//Get format version
		uint16_t version = 0;
		stream.read(reinterpret_cast<char*>(&version), 2);

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
			unsigned int bzOutSizeSink = uncompressedSize;
			int status = BZ2_bzBuffToBuffDecompress(reinterpret_cast<char*>(uncompressed.data()), &bzOutSizeSink, reinterpret_cast<char*>(compressed.data()), compressedSize, 0, 0);
			CheckException(status == BZ_OK, "Failed to decompress packed container payload!");
		}

		//Create output
		PackedContainer out(format, version, std::move(uncompressed));

		//Return output
		return out;
	}

	void PackedContainer::ExportToStream(std::ostream& stream) {
		CheckException(stream.good(), "Output stream for packed container export is invalid!");

		//Write Cacao Engine magic number (written backwards for endianness)
		constexpr unsigned int magic = 0x00CACA;
		stream.write(reinterpret_cast<const char*>(&magic), 3);

		//Write format code
		unsigned char code = 0;
		switch(format) {
			case PackedFormat::AssetPack:
				code = 0xAA;
				break;
			case PackedFormat::Cubemap:
				code = 0xC4;
				break;
			case PackedFormat::Material:
				code = 0x3E;
				break;
			case PackedFormat::Shader:
				code = 0x1B;
				break;
			case PackedFormat::World:
				code = 0x7A;
				break;
		}
		stream.write(reinterpret_cast<char*>(&code), 1);

		//Write version
		stream.write(reinterpret_cast<const char*>(&version), 2);

		//Write buffer size
		std::size_t bufSize = payload.size() * sizeof(unsigned char);
		stream.write(reinterpret_cast<char*>(&bufSize), sizeof(std::size_t));

		//Compress payload
		std::vector<unsigned char> compressed(bufSize * 1.01 + 600);
		unsigned int destSize = compressed.size();																																		  //This size ratio comes from the bzip2 docs
		int status = BZ2_bzBuffToBuffCompress(reinterpret_cast<char*>(compressed.data()), &destSize, const_cast<char*>(reinterpret_cast<const char*>(payload.data())), bufSize, 9, 0, 30);//9 is the compression level, which is the default from the bzip2 command-line tool. 30 is the recommended work factor by the bzip2 docs.
		CheckException(status == BZ_OK, "Failed to compress payload for packed container export!");

		//Trim compressed data to free any memory not used
		compressed.resize(destSize);

		//Write compressed data
		stream.write(reinterpret_cast<char*>(compressed.data()), compressed.size());

		//Flush output
		stream << std::flush;
	}
}