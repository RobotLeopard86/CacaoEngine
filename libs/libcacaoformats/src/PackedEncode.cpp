#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"

namespace libcacaoformats {
	PackedContainer PackedEncoder::EncodeCubemap(const std::array<ImageBuffer, 6>& cubemap) {
		//Validate input
		uint64_t sizes[] = {0, 0, 0, 0, 0, 0};
		uint8_t i = 0;
		for(const ImageBuffer& buf : cubemap) {
			CheckException(buf.channelCount > 0 && buf.size.x > 0 && buf.size.y > 0, "Cubemap face has invalid dimensions or channel count!");
			CheckException(!buf.data.empty(), "Cubemap face has empty data buffer!");
			sizes[i++] = (uint64_t)buf.size.x * (uint64_t)buf.size.y * (uint64_t)buf.channelCount;
		}

		//Create output container
		std::vector<char> outBuffer;
		obytestream out(outBuffer);

		//Write face sizes
		out.write(reinterpret_cast<char*>(&sizes[0]), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(&sizes[1]), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(&sizes[2]), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(&sizes[3]), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(&sizes[4]), sizeof(uint64_t));
		out.write(reinterpret_cast<char*>(&sizes[5]), sizeof(uint64_t));

		//Encode face data and write it out
		EncodeImage(cubemap[0], out);
		EncodeImage(cubemap[1], out);
		EncodeImage(cubemap[2], out);
		EncodeImage(cubemap[3], out);
		EncodeImage(cubemap[4], out);
		EncodeImage(cubemap[5], out);

		//Create and return packed container
		return PackedContainer(FormatCode::Cubemap, 1, std::move(outBuffer));
	}
}