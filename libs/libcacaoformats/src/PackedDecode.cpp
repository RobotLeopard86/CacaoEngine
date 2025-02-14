#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"

#include <cstdint>
#include <cstring>

namespace libcacaoformats {
	std::array<ImageBuffer, 6> PackedDecoder::DecodeCubemap(const PackedContainer& container) {
		CheckException(container.format == FormatCode::Cubemap, "Packed container provided for cubemap decoding is not a cubemap!");
		CheckException(container.payload.size() > 28, "Cubemap packed container is too small to contain face size data!");

		//Get buffer sizes
		uint32_t pxs, nxs, pys, nys, pzs, nzs;
		std::memcpy(&pxs, container.payload.data(), 4);
		std::memcpy(&nxs, container.payload.data() + 4, 4);
		std::memcpy(&pys, container.payload.data() + 8, 4);
		std::memcpy(&nys, container.payload.data() + 12, 4);
		std::memcpy(&pzs, container.payload.data() + 16, 4);
		std::memcpy(&nzs, container.payload.data() + 24, 4);

		//Extract encoded face buffers
		std::vector<char> pxFB(pxs), nxFB(nxs), pyFB(pys), nyFB(nys), pzFB(pzs), nzFB(nzs);
		std::size_t offsetCounter = 28;
		CheckException(container.payload.size() > (28 + pxs + nxs + pys + nys + pzs + nzs), "Cubemap packed container is too small to contain face data of given sizes!");
		std::memcpy(pxFB.data(), container.payload.data() + offsetCounter, pxs);
		offsetCounter += pxs;
		std::memcpy(nxFB.data(), container.payload.data() + offsetCounter, nxs);
		offsetCounter += nxs;
		std::memcpy(pyFB.data(), container.payload.data() + offsetCounter, pys);
		offsetCounter += pys;
		std::memcpy(nyFB.data(), container.payload.data() + offsetCounter, nys);
		offsetCounter += nys;
		std::memcpy(pzFB.data(), container.payload.data() + offsetCounter, pzs);
		offsetCounter += pzs;
		std::memcpy(nzFB.data(), container.payload.data() + offsetCounter, nzs);
		offsetCounter += nzs;

		//Decode face buffers
		std::array<ImageBuffer, 6> out {};
		ibytestream pxStream(pxFB);
		out[0] = DecodeImage(pxStream);
		ibytestream nxStream(nxFB);
		out[1] = DecodeImage(nxStream);
		ibytestream pyStream(pyFB);
		out[2] = DecodeImage(pyStream);
		ibytestream nyStream(nyFB);
		out[3] = DecodeImage(nyStream);
		ibytestream pzStream(pzFB);
		out[4] = DecodeImage(pzStream);
		ibytestream nzStream(nzFB);
		out[5] = DecodeImage(nzStream);

		//Return result
		return out;
	}

	Shader PackedDecoder::DecodeShader(const PackedContainer& container) {
		CheckException(container.format == FormatCode::Shader, "Packed container provided for shader decoding is not a shader!");
		CheckException(container.payload.size() > 8, "Shader packed container is too small to contain code size data!");

		//Get buffer sizes
		uint32_t vscsz, pscsz;
		std::memcpy(&vscsz, container.payload.data(), 4);
		std::memcpy(&pscsz, container.payload.data() + 4, 4);

		//Extract code buffers
		Shader shader {};
		shader.vertexSPV.reserve(vscsz);
		shader.fragmentSPV.reserve(pscsz);
		std::memcpy(shader.vertexSPV.data(), container.payload.data() + 8, vscsz);
		std::memcpy(shader.fragmentSPV.data(), container.payload.data() + 8 + vscsz, pscsz);

		//Return result
		return shader;
	}

	Material PackedDecoder::DecodeMaterial(const PackedContainer& container) {
		CheckException(container.format == FormatCode::Material, "Packed container provided for material decoding is not a material!");
		CheckException(container.payload.size() > 2, "Material packed container is too small to contain shader address string size data!");

		Material out {};

		//Get shader address string
		uint16_t saLen = 0;
		std::memcpy(&saLen, container.payload.data(), 2);
		CheckException(saLen > 0, "Material packed container has zero-length shader address string");
		CheckException(container.payload.size() > 3 + saLen, "Material packed container is too small to contain shader address string!");
		out.shader = std::string('\0', saLen);
		std::memcpy(out.shader.data(), container.payload.data() + 2, saLen);

		//Get material keys count
		uint8_t numKeys = 0;
		std::memcpy(&numKeys, container.payload.data() + 2 + saLen, 1);

		//If we don't need to process keys, return value now
		if(numKeys == 0) return out;

		//Process keys
		std::size_t offsetCounter = saLen + 3;
		for(uint8_t i = 0; i < numKeys; i++) {
			//Get type info
			CheckException(container.payload.size() > offsetCounter + 1, "Material packed container is too small to contain provided key count!");
			uint8_t typeInfo = 0;
			std::memcpy(&typeInfo, container.payload.data() + offsetCounter++, 1);

			//Extract info and check size constraints
			Vec2<uint8_t> size;
			size.x = (typeInfo & 0b00001100);
			size.y = (typeInfo & 0b00110000);
			uint8_t baseType = (typeInfo & 0b00000011);
			CheckException(baseType == 3 && (size.x > 1 || size.y > 1), "Material packed container key has invalid size for texture type (must be 1x1)!");
			CheckException(baseType != 2 && size.y > 1, "Material packed container key has invalid size for non-float type (y must be 1)!");
			CheckException(size.x == 1 && size.y != 1, "Material packed container key has invalid size (if x is 1, must be 1x1)");
			uint8_t dims = (4 * size.y) - (4 - size.x);
		}
	}
}