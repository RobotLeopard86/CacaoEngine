#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"

namespace libcacaoformats {
	PackedContainer PackedEncoder::EncodeCubemap(const std::array<ImageBuffer, 6>& cubemap) {
		//Validate input
		uint64_t sizes[] = {0, 0, 0, 0, 0, 0};
		uint8_t i = 0;
		for(const ImageBuffer& buf : cubemap) {
			CheckException(buf.channelCount > 0 && buf.size.x > 0 && buf.size.y > 0, "Cubemap face for packed encoding has invalid dimensions or channel count!");
			CheckException(!buf.data.empty(), "Cubemap face for packed encoding has empty data buffer!");
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

	PackedContainer PackedEncoder::EncodeShader(const Shader& shader) {
		//Validate input
		CheckException(shader.vertexSPV.size() > 0, "Shader for packed encoding has zero-size vertex SPIR-V!");
		CheckException(shader.fragmentSPV.size() > 0, "Shader for packed encoding has zero-size fragment SPIR-V!");

		//Create output container
		std::vector<char> outBuffer;
		obytestream out(outBuffer);

		//Write face sizes
		uint32_t vertexSize = shader.vertexSPV.size() * sizeof(uint32_t);
		uint32_t fragmentSize = shader.vertexSPV.size() * sizeof(uint32_t);
		out.write(reinterpret_cast<char*>(&vertexSize), sizeof(uint32_t));
		out.write(reinterpret_cast<char*>(&fragmentSize), sizeof(uint32_t));

		//Write SPIR-V
		out.write(reinterpret_cast<const char*>(shader.vertexSPV.data()), vertexSize);
		out.write(reinterpret_cast<const char*>(shader.fragmentSPV.data()), fragmentSize);

		//Create and return packed container
		return PackedContainer(FormatCode::Cubemap, 1, std::move(outBuffer));
	}
}