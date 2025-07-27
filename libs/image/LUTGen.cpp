#include <array>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <iostream>

#define SRGB_LINEAR_THRESHOLD 0.0031308f
#define SRGB_LINEAR_MULTIPLIER 12.92f
#define SRGB_EXPONENT 1.0f / 2.4f
#define SRGB_SCALE 1.055f
#define SRGB_OFFSET 0.055f

//sRGB conversion
float toSrgb(float lin) {
	return (lin <= SRGB_LINEAR_THRESHOLD) ? lin * SRGB_LINEAR_MULTIPLIER : SRGB_SCALE * std::pow(lin, SRGB_EXPONENT) - SRGB_OFFSET;
}

//LUT generator
std::array<uint8_t, 65536> lutGenerator() {
	std::array<uint8_t, 65536> lut = {};
	for(uint32_t i = 0; i < 65536; ++i) {
		float normalized = static_cast<float>(i) / 65535.0f;
		float srgb = toSrgb(normalized);
		lut[i] = static_cast<uint8_t>(std::clamp(srgb * 255.0f, 0.0f, 255.0f));
	}
	return lut;
}

int main(int argc, char* argv[]) {
	if(argc < 1) exit(-1);
	if(argc != 2) {
		std::cerr << "Wrong argument count.\n\nUsage: " << argv[0] << " <output file>" << std::endl;
		exit(-1);
	}

	//Generate LUT
	const auto lut = lutGenerator();

	//Write output
	std::ofstream out(argv[1]);
	out << "#pragma once\n\n#include <cstdint>\n\ninline constexpr uint8_t lut[65536] = {\n";
	for(size_t i = 0; i < lut.size(); ++i) {
		out << static_cast<unsigned>(lut[i]);
		if(i != lut.size() - 1) out << ", ";
		if((i + 1) % 16 == 0) out << "\n";
	}
	out << "};\n";
}