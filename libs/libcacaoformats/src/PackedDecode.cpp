#include "libcacaoformats/libcacaoformats.hpp"

#include "CheckException.hpp"

#include <cstdint>

namespace libcacaoformats {
	std::array<ImageBuffer, 6> PackedDecoder::DecodeCubemap(const PackedContainer& container) {
		CheckException(container.format == FormatCode::Cubemap, "Packed container provided for cubemap decoding is not a cubemap!");

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
}